#include "state.h"

#include <QtConcurrent>
#include <QHttpMultiPart>
#include <QRandomGenerator>
#include <chrono>

#include "messages_model_p.h"
#include "overrides_model_p.h"
#include "uploading.h"

enum Roles {
	ID,
	Next,
	Previous,
};

MessagesModel::MessagesModel(QString host, quint64 guildID, quint64 channelID, State* state) : QAbstractListModel(state), d(new Private), host(host), s(state)
{
	d->store.reset(new MessagesStore(this, state, host));
	d->guildID = guildID;
	d->channelID = channelID;
	d->typingTimer = new QTimer(this);
	d->typingTimer->setInterval(std::chrono::milliseconds(5000));
	d->typingTimer->setSingleShot(true);

	state->api()->subscribeToGuild(host, guildID);
	connect(state->api(), &SDK::ClientManager::chatEvent, this, [=](QString hs, protocol::chat::v1::StreamEvent ev) {
		using namespace protocol::chat::v1;
		if (hs != host) {
			return;
		}

		switch (ev.event_case()) {
		case StreamEvent::kTyping: {
			auto sm = ev.typing();
			if (sm.guild_id() != d->guildID || sm.channel_id() != d->channelID) {
				return;
			}

			const auto user = sm.user_id();
			if (d->typingTimers.contains(user)) {
				d->typingTimers[user]->start();

				return;
			}

			auto timer = new QTimer(this);
			timer->setInterval(std::chrono::milliseconds(5500));
			timer->setSingleShot(true);
			connect(timer, &QTimer::timeout, [timer, user, this]() {
				d->typingTimers.remove(user);
				d->typingUsers.removeAll(user);
				timer->deleteLater();

				Q_EMIT nowTypingChanged();
			});

			d->typingTimers[user] = timer;
			d->typingUsers << user;
			timer->start();
			Q_EMIT nowTypingChanged();
			break;
		}
		case StreamEvent::kSentMessage: {
			auto sm = ev.sent_message();
			auto it = sm.message();
			if (sm.guild_id() != d->guildID || sm.channel_id() != d->channelID) {
				return;
			}
			if (sm.has_echo_id()) {
				const auto eid = MessageID { MessageID::Echo, sm.echo_id() };
				auto idx = d->messageIDs.indexOf(eid);
				beginRemoveRows(QModelIndex(), idx, idx);
				d->messageIDs.removeAll(eid);
				endRemoveRows();
			}
			beginInsertRows(QModelIndex(), 0, 0);
			d->messageIDs.prepend(MessageID { MessageID::Remote, sm.message_id() });
			d->store->newMessage(sm.message_id(), it);
			endInsertRows();
			dataChanged(index(1), index(1));
			break;
		}
		case StreamEvent::kDeletedMessage: {
			const auto& dm = ev.deleted_message();
			if (dm.guild_id() != d->guildID || dm.channel_id() != d->channelID) {
				return;
			}
			auto idx = d->messageIDs.indexOf(MessageID { MessageID::Remote, dm.message_id() });
			beginRemoveRows(QModelIndex(), idx, idx);
			d->messageIDs.removeAll(MessageID { MessageID::Remote, dm.message_id() });
			d->store->deleteMessage(dm.message_id());
			endRemoveRows();
			break;
		}
		case StreamEvent::kEditedMessage: {
			const auto& em = ev.edited_message();
			if (em.guild_id() != d->guildID || em.channel_id() != d->channelID) {
				return;
			}
			d->store->editMessage(em.channel_id(), em.new_content());
			break;
		}
		}
	});
}

MessagesModel::~MessagesModel()
{
}

bool MessagesModel::canFetchMore(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return d->canFetchMore && !d->isFetching;
}

Croutons::FutureBase MessagesModel::typing()
{
	protocol::chat::v1::TypingRequest req;
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);

	auto resp = co_await s->api()->dispatch(host, &SDK::R::Typing, req);
	Q_UNUSED(resultOk(resp));
	
	co_return {};
}

void MessagesModel::doTyping()
{
	if (d->typingTimer->isActive()) {
		return;
	}

	typing();
	d->typingTimer->start();
}

QStringList MessagesModel::nowTyping()
{
	QStringList ret;

	for (auto user : d->typingUsers) {
		ret << QString::number(user);
	}

	return ret;
}

void MessagesModel::fetchMore(const QModelIndex &parent)
{
	Q_UNUSED(parent)

	if (d->isFetching) {
		return;
	}

	if (d->guildID == 0 || d->channelID == 0) {
		return;
	}

	protocol::chat::v1::GetChannelMessagesRequest req;
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);
	if (!d->messageIDs.isEmpty()) {
		req.set_message_id(d->messageIDs.last().id);
		req.set_direction(protocol::chat::v1::GetChannelMessagesRequest::Direction::GetChannelMessagesRequest_Direction_DIRECTION_BEFORE_UNSPECIFIED);
	}

	d->isFetching = true;

	s->api()->dispatch(host, &SDK::R::GetChannelMessages, req).then([this, req](auto r) {
		d->isFetching = false;

		if (!resultOk(r)) {
			return;
		}

		protocol::chat::v1::GetChannelMessagesResponse resp = unwrap(r);

		d->canFetchMore = !resp.reached_top();

		beginInsertRows(QModelIndex(), d->messageIDs.length(), (d->messageIDs.length()+resp.messages_size())-1);
		for (auto item : resp.messages()) {
			d->messageIDs << MessageID { MessageID::Remote, item.message_id() };
			d->store->newMessage(item.message_id(), item.message());
		}
		endInsertRows();
	});
}

int MessagesModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->messageIDs.length();
}

QVariant MessagesModel::data(const QModelIndex& index, int role) const
{
	auto r = index.row();
	if (r >= d->messageIDs.length()) {
		return QVariant();
	}

	switch (role) {
	case Roles::ID:
		return d->messageIDs[r].toString();
	case Roles::Next:
		if (r-1 < d->messageIDs.length() and r-1 >= 0) {
			return d->messageIDs[r-1].toString();
		}
		return QString();
	case Roles::Previous:
		if (r+1 < d->messageIDs.length()) {
			return d->messageIDs[r+1].toString();
		}
		return QString();
	}

	return QVariant();
}

MessagesStore* MessagesModel::store()
{
	return d->store.get();
}

void MessagesModel::echoMessage(protocol::chat::v1::SendMessageRequest& req)
{
	const auto echo = QRandomGenerator::system()->generate64();
	req.set_echo_id(echo);
	d->store->echoMessage(echo, req);

	beginInsertRows(QModelIndex(), 0, 0);
	d->messageIDs.prepend(MessageID { MessageID::Echo, echo });
	endInsertRows();
}

FutureBase MessagesModel::send(QString txt, QVariant override, QString inReplyTo)
{
	protocol::chat::v1::SendMessageRequest req;
	req.set_in_reply_to(inReplyTo.toULongLong());
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);
	req.set_allocated_content(new protocol::chat::v1::Content);
	req.mutable_content()->set_allocated_text_message(new protocol::chat::v1::Content::TextContent);
	req.mutable_content()->mutable_text_message()->set_allocated_content(new protocol::chat::v1::FormattedText);
	req.mutable_content()->mutable_text_message()->mutable_content()->set_text(txt.toStdString());
	if (!override.isNull()) {
		const auto data = override.value<OverrideData>();
		req.set_allocated_overrides(new protocol::chat::v1::Overrides);
		req.mutable_overrides()->set_username(data.name.toStdString());
		if (!data.avatar.isEmpty()) {
			req.mutable_overrides()->set_avatar(data.avatar.toStdString());
		}
		req.mutable_overrides()->set_allocated_system_plurality(new protocol::harmonytypes::v1::Empty);
	}
	for (const auto& override : s->overridesModel()->d->overrides) {
		for (const auto& tag : override.tags()) {
			const auto before = QString::fromStdString(tag.before());
			const auto evalBefore = !before.isEmpty();
			const auto beforeOK = evalBefore ? txt.startsWith(before) : true;
			const auto after = QString::fromStdString(tag.after());
			const auto evalAfter = !after.isEmpty();
			const auto afterOK = evalAfter ? txt.endsWith(after) : true;

			if (!(beforeOK && afterOK)) {
				continue;
			}

			req.set_allocated_overrides(new protocol::chat::v1::Overrides);
			req.mutable_overrides()->set_username(override.username());
			if (!override.avatar().empty()) {
				req.mutable_overrides()->set_avatar(override.avatar());
			}
			req.mutable_overrides()->set_allocated_system_plurality(new protocol::harmonytypes::v1::Empty);

			txt.remove(0, before.length());
			txt.chop(after.length());

			req.mutable_content()->mutable_text_message()->mutable_content()->set_text(txt.toStdString());
		}
	}
	echoMessage(req);
	co_await s->api()->dispatch(host, &SDK::R::SendMessage, req);
	co_return QVariant();
}

FutureBase MessagesModel::sendDebugMessage()
{
	using namespace protocol::chat::v1;
	SendMessageRequest req;
	req.set_allocated_content(new Content);
	req.mutable_content()->set_allocated_text_message(new Content::TextContent);

	const auto tm = req.mutable_content()->mutable_text_message();
	tm->set_allocated_content(new FormattedText);

	const auto txt = tm->mutable_content();
	const auto cont = R"(bold text
italic text
underline text
monospace text
superscript text
subscript text
codeblock text
usermention text
rolemention text
channelmention text
guildmention text
emoji text
color dim text
color bright text
color negative text
color positive text
color info text
color warning text
)";
	const QString content(cont);
	txt->set_text(cont);
	auto addfmt = [txt, &content](const QString& it, Format fmt)
	{
		fmt.set_start(content.indexOf(it));
		fmt.set_length(it.length());
		*(txt->add_format()) = fmt;
	};

	Format it;
	it.set_allocated_bold(new Format::Bold);
	addfmt("bold", it);

	it.set_allocated_italic(new Format::Italic);
	addfmt("italic", it);

	it.set_allocated_underline(new Format::Underline);
	addfmt("underline", it);

	it.set_allocated_monospace(new Format::Monospace);
	addfmt("monospace", it);

	it.set_allocated_superscript(new Format::Superscript);
	addfmt("superscript", it);

	it.set_allocated_subscript(new Format::Subscript);
	addfmt("subscript", it);

	it.set_allocated_code_block(new Format::CodeBlock);
	addfmt("codeblock", it);

	it.set_allocated_user_mention(new Format::UserMention);
	addfmt("usermention", it);

	it.set_allocated_role_mention(new Format::RoleMention);
	addfmt("rolemention", it);

	it.set_allocated_channel_mention(new Format::ChannelMention);
	addfmt("channelmention", it);

	it.set_allocated_guild_mention(new Format::GuildMention);
	addfmt("guildmention", it);

	it.set_allocated_emoji(new Format::Emoji);
	it.mutable_emoji()->set_image_hmc("AHyrDmaB3yGZC5UjGYO92n");
	addfmt("emoji", it);

	it.set_allocated_color(new Format::Color);
	it.mutable_color()->set_kind(Format::Color::KIND_DIM_UNSPECIFIED);
	addfmt("color dim", it);

	it.set_allocated_color(new Format::Color);
	it.mutable_color()->set_kind(Format::Color::KIND_BRIGHT);
	addfmt("color bright", it);

	it.set_allocated_color(new Format::Color);
	it.mutable_color()->set_kind(Format::Color::KIND_NEGATIVE);
	addfmt("color negative", it);

	it.set_allocated_color(new Format::Color);
	it.mutable_color()->set_kind(Format::Color::KIND_POSITIVE);
	addfmt("color positive", it);

	it.set_allocated_color(new Format::Color);
	it.mutable_color()->set_kind(Format::Color::KIND_INFO);
	addfmt("color info", it);

	it.set_allocated_color(new Format::Color);
	it.mutable_color()->set_kind(Format::Color::KIND_WARNING);
	addfmt("color warning", it);

	// it.set_allocated_localization(new Format::Localization);
	// addfmt("localisation", it);

	it.set_allocated_bold(new Format::Bold);
	addfmt("bold", it);

	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);

	echoMessage(req);
	co_await s->api()->dispatch(host, &SDK::R::SendMessage, req);
	co_return QVariant();
}

FutureBase MessagesModel::sendFiles(const QList<QUrl>& urls)
{
	auto rids = co_await uploadFiles(s, host, urls);
	if (!rids.ok()) {
		co_return QVariant();
	}

	auto ids = rids.value();

	protocol::chat::v1::SendMessageRequest req;
	req.set_allocated_content(new protocol::chat::v1::Content);
	req.mutable_content()->set_allocated_attachment_message(new protocol::chat::v1::Content::AttachmentContent);
	for (const auto& it : ids) {
		protocol::chat::v1::Attachment attach;
		attach.set_id(it.toStdString());
		req.mutable_content()->mutable_attachment_message()->mutable_files()->Add(std::move(attach));
	}

	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);

	echoMessage(req);
	co_await s->api()->dispatch(host, &SDK::R::SendMessage, req);
	co_return QVariant();
}

QHash<int,QByteArray> MessagesModel::roleNames() const
{
	return {
		{ ID, "messageID" },
		{ Next, "nextMessageID" },
		{ Previous, "previousMessageID" },
	};
}

FutureBase MessagesModel::deleteMessage(const QString& id)
{
	using namespace protocol::chat::v1;

	DeleteMessageRequest req;
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);
	req.set_message_id(id.toULongLong());

	auto it = co_await s->api()->dispatch(host, &SDK::R::DeleteMessage, req);

	co_return it.ok();
}
