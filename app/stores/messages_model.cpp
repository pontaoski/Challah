#include "state.h"

#include <QtConcurrent>
#include <QHttpMultiPart>

#include "messages_model_p.h"
#include "coroutine_integration_network.h"

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

	state->api()->subscribeToGuild(host, guildID);
	connect(state->api(), &SDK::ClientManager::chatEvent, this, [=](QString hs, protocol::chat::v1::StreamEvent ev) {
		using namespace protocol::chat::v1;
		if (hs != host) {
			return;
		}

		switch (ev.event_case()) {
		case StreamEvent::kSentMessage: {
			beginInsertRows(QModelIndex(), 0, 0);
			auto sm = ev.sent_message();
			auto it = sm.message();
			if (sm.guild_id() != d->guildID || sm.channel_id() != d->channelID) {
				return;
			}
			d->messageIDs.prepend(sm.message_id());
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
			auto idx = d->messageIDs.indexOf(dm.message_id());
			beginRemoveRows(QModelIndex(), idx, idx);
			d->messageIDs.removeAll(dm.message_id());
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

void MessagesModel::fetchMore(const QModelIndex &parent)
{
	Q_UNUSED(parent)

	if (d->isFetching) {
		return;
	}

	protocol::chat::v1::GetChannelMessagesRequest req;
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);
	if (!d->messageIDs.isEmpty()) {
		req.set_message_id(d->messageIDs.last());
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
			d->messageIDs << item.message_id();
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
		return QString::number(d->messageIDs[r]);
	case Roles::Next:
		if (r-1 < d->messageIDs.length() and r-1 >= 0) {
			return QString::number(d->messageIDs[r-1]);
		}
		return QString();
	case Roles::Previous:
		if (r+1 < d->messageIDs.length()) {
			return QString::number(d->messageIDs[r+1]);
		}
		return QString();
	}

	return QVariant();
}

MessagesStore* MessagesModel::store()
{
	return d->store.get();
}

FutureBase MessagesModel::send(QString txt, QString inReplyTo)
{
	protocol::chat::v1::SendMessageRequest req;
	req.set_in_reply_to(inReplyTo.toULongLong());
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);
	req.set_allocated_content(new protocol::chat::v1::Content);
	req.mutable_content()->set_allocated_text_message(new protocol::chat::v1::Content::TextContent);
	req.mutable_content()->mutable_text_message()->set_allocated_content(new protocol::chat::v1::FormattedText);
	req.mutable_content()->mutable_text_message()->mutable_content()->set_text(txt.toStdString());
	co_await s->api()->dispatch(host, &SDK::R::SendMessage, req);
	co_return QVariant();
}

FutureBase MessagesModel::sendFiles(const QList<QUrl>& urls)
{
	QStringList ids;

	for (const auto& url : urls) {
		QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

		QFile* file(new QFile(url.toLocalFile()));
		file->open(QIODevice::ReadOnly);

		QHttpPart filePart;
		filePart.setBodyDevice(file);
		filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"file\""));
		filePart.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");

		mp->append(filePart);

		QUrlQuery query;
		query.addQueryItem("filename", url.fileName());
		query.addQueryItem("contentType", QMimeDatabase().mimeTypeForFile(url.toLocalFile()).name());

		auto c = co_await s->api()->clientForHomeserver(host);

		QUrl reqUrl(c->homeserver() + "/_harmony/media/upload?" + query.query());
		QNetworkRequest req(reqUrl);
		req.setRawHeader("Authorization", QByteArray::fromStdString(c->session()));
		QNetworkAccessManager nam;

		const auto reply = co_await nam.post(req, mp);
		const auto id = QJsonDocument::fromJson(reply->readAll())["id"].toString();
		ids << id;
	}

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
