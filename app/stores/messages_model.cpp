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

MessagesModel::MessagesModel(SDK::Client* client, quint64 guildID, quint64 channelID, State* state) : QAbstractListModel(state), d(new Private), c(client)
{
	d->store.reset(new MessagesStore(this, client));
	d->guildID = guildID;
	d->channelID = channelID;

	state->api()->subscribeToGuild(client->homeserver(), guildID);
	connect(state->api(), &SDK::ClientManager::chatEvent, this, [=](QString hs, protocol::chat::v1::Event ev) {
		using namespace protocol::chat::v1;
		if (hs != c->homeserver()) {
			return;
		}

		switch (ev.event_case()) {
		case Event::kSentMessage: {
			beginInsertRows(QModelIndex(), 0, 0);
			auto it = ev.sent_message().message();
			if (it.guild_id() != d->guildID || it.channel_id() != d->channelID) {
				return;
			}
			d->messageIDs.prepend(it.message_id());
			d->store->newMessage(it.message_id(), it);
			endInsertRows();
			dataChanged(index(1), index(1));
		}
		case Event::kDeletedMessage:
			;
		case Event::kEditedMessage:
			;
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
		req.set_before_message(d->messageIDs.last());
	}

	d->isFetching = true;

	c->chatKit()->GetChannelMessages(req).then([this, req](auto r) {
		d->isFetching = false;

		if (!resultOk(r)) {
			return;
		}

		protocol::chat::v1::GetChannelMessagesResponse resp = unwrap(r);

		d->canFetchMore = !resp.reached_top();

		beginInsertRows(QModelIndex(), d->messageIDs.length(), (d->messageIDs.length()+resp.messages_size())-1);
		for (auto item : resp.messages()) {
			d->messageIDs << item.message_id();
			d->store->newMessage(item.message_id(), item);
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

FutureBase MessagesModel::send(QString txt)
{
	protocol::chat::v1::SendMessageRequest req;
	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);
	req.set_allocated_content(new protocol::harmonytypes::v1::Content);
	req.mutable_content()->set_allocated_text_message(new protocol::harmonytypes::v1::ContentText);
	req.mutable_content()->mutable_text_message()->set_content(txt.toStdString());
	co_await c->chatKit()->SendMessage(req);
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

		QUrl reqUrl(c->homeserver() + "/_harmony/media/upload?" + query.query());
		QNetworkRequest req(reqUrl);
		req.setRawHeader("Authorization", QByteArray::fromStdString(c->session()));
		QNetworkAccessManager nam;

		const auto reply = co_await nam.post(req, mp);
		const auto id = QJsonDocument::fromJson(reply->readAll())["id"].toString();
		ids << id;
	}

	protocol::chat::v1::SendMessageRequest req;
	req.set_allocated_content(new protocol::harmonytypes::v1::Content);
	req.mutable_content()->set_allocated_files_message(new protocol::harmonytypes::v1::ContentFiles);
	for (const auto& it : ids) {
		protocol::harmonytypes::v1::Attachment attach;
		attach.set_id(it.toStdString());
		req.mutable_content()->mutable_files_message()->mutable_attachments()->Add(std::move(attach));
	}

	req.set_guild_id(d->guildID);
	req.set_channel_id(d->channelID);

	co_await c->chatKit()->SendMessage(req);
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
