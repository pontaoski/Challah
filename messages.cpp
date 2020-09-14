#include "channels.hpp"
#include "messages.hpp"

using grpc::ClientContext;

MessagesModel::MessagesModel(ChannelsModel *parent, QString homeServer, quint64 guildID, quint64 channelID)
	: QAbstractListModel((QObject*)parent),
		guildID(guildID),
		channelID(channelID),
		homeServer(homeServer)
{
	client = Client::instanceForHomeserver(homeServer);
}

void MessagesModel::customEvent(QEvent *event)
{
	if (event->type() == MessageSentEvent::typeID) {
		auto ev = reinterpret_cast<MessageSentEvent*>(event);
		auto msg = ev->data.message();
		auto idx = messageData.count();
		beginInsertRows(QModelIndex(), 0, 0);
		messageData.push_front(MessageData::fromProtobuf(msg));
		endInsertRows();
	}
}

int MessagesModel::rowCount(const QModelIndex& parent) const
{
	return messageData.count();
}

QVariant MessagesModel::data(const QModelIndex& index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	auto idx = index.row();

	switch (role)
	{
	case MessageTextRole:
		return messageData[idx].text;
	case MessageAuthorRole:
		return qobject_cast<ChannelsModel*>(parent())->userName(messageData[idx].authorID);
	case MessageDateRole:
		return messageData[idx].date;
	}

	return QVariant();
}

QHash<int,QByteArray> MessagesModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[MessageTextRole] = "content";
	ret[MessageAuthorRole] = "authorName";
	ret[MessageDateRole] = "date";

	return ret;
}

void MessagesModel::sendMessage(const QString& message)
{
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::SendMessageRequest req;

	req.set_allocated_location(Location {
		.guildID = guildID,
		.channelID = channelID
	});
	req.set_content(message.toStdString());

	google::protobuf::Empty empty;

	client->coreKit->SendMessage(&ctx, req, &empty);
}

bool MessagesModel::canFetchMore(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return !atEnd;
}

void MessagesModel::fetchMore(const QModelIndex& parent)
{
	Q_UNUSED(parent)

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::GetChannelMessagesRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID,
		.channelID = channelID
	});
	protocol::core::v1::GetChannelMessagesResponse resp;

	if (!messageData.isEmpty()) {
		req.set_before_message(messageData.last().id);
	}

	if (checkStatus(client->coreKit->GetChannelMessages(&ctx, req, &resp))) {
		if (resp.messages_size() == 0) {
			atEnd = true;
			return;
		}

		beginInsertRows(QModelIndex(), messageData.count(), (messageData.count()+resp.messages_size())-1);
		for (auto item : resp.messages()) {
			messageData << MessageData::fromProtobuf(item);
		}
		endInsertRows();
	}
}
