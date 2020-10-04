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

	{
		ClientContext ctx;
		client->authenticate(ctx);

		protocol::core::v1::GetGuildRequest req;
		req.set_allocated_location(Location {
			.guildID = guildID
		});
		protocol::core::v1::GetGuildResponse resp;
		if (checkStatus(client->coreKit->GetGuild(&ctx, req, &resp))) {
			isGuildOwner = client->userID == resp.guild_owner();
		}
	}
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
	} else if (event->type() == MessageUpdatedEvent::typeID) {
		auto ev = reinterpret_cast<MessageUpdatedEvent*>(event);
		auto& msg = ev->data;
		auto idx = -1;
		for (auto& message : messageData) {
			idx++;
			if (message.id == msg.location().message_id()) {
				message.editedAt = QDateTime::fromTime_t(msg.edited_at().seconds());

				std::string jsonified;
				google::protobuf::util::MessageToJsonString(msg, &jsonified, google::protobuf::util::JsonPrintOptions{});
				auto document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonified));

				if (msg.update_actions()) {
					message.actions = document["actions"];
				}
				if (msg.update_attachments()) {
					// we don't do attachments rn
				}
				if (msg.update_content()) {
					message.text = QString::fromStdString(msg.content());
				}
				if (msg.update_embeds()) {
					message.embeds = document["embeds"];
				}
				dataChanged(index(idx), index(idx));
				break;
			}
		}
	} else if (event->type() == MessageDeletedEvent::typeID) {
		auto ev = reinterpret_cast<MessageDeletedEvent*>(event);
		auto msg = ev->data.location().message_id();
		auto idx = -1;
		for (auto& message : messageData) {
			if (message.id == msg) {
				idx++;
				break;
			}
			idx++;
		}
		if (idx != -1) {
			beginRemoveRows(QModelIndex(), idx, idx);
			messageData.removeAt(idx);
			endRemoveRows();
		}
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
	case MessageAuthorIDRole:
		return QString::number(messageData[idx].authorID);
	case MessageDateRole:
		return messageData[idx].date;
	case MessageEmbedsRole:
		return messageData[idx].embeds;
	case MessageActionsRole:
		return messageData[idx].actions;
	case MessageIDRole:
		return QString::number(messageData[idx].id);
	}

	return QVariant();
}

QHash<int,QByteArray> MessagesModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[MessageTextRole] = "content";
	ret[MessageAuthorRole] = "authorName";
	ret[MessageAuthorIDRole] = "authorID";
	ret[MessageDateRole] = "date";
	ret[MessageEmbedsRole] = "embeds";
	ret[MessageActionsRole] = "actions";
	ret[MessageIDRole] = "messageID";

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

void MessagesModel::triggerAction(const QString& messageID, const QString &name, const QString &data)
{
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::TriggerActionRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID,
		.channelID = channelID,
		.messageID = messageID.toULongLong()
	});
	req.set_action_id(name.toStdString());
	if (data != QString()) {
		req.set_action_data(data.toStdString());
	}
	google::protobuf::Empty resp;

	checkStatus(client->coreKit->TriggerAction(&ctx, req, &resp));
}

void MessagesModel::editMessage(const QString& id, const QString &content)
{
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::UpdateMessageRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID,
		.channelID = channelID,
		.messageID = id.toULongLong()
	});
	req.set_content(content.toStdString());
	req.set_update_content(true);
	google::protobuf::Empty resp;

	checkStatus(client->coreKit->UpdateMessage(&ctx, req, &resp));
}

void MessagesModel::deleteMessage(const QString& id)
{
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::DeleteMessageRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID,
		.channelID = channelID,
		.messageID = id.toULongLong()
	});
	google::protobuf::Empty resp;

	checkStatus(client->coreKit->DeleteMessage(&ctx, req, &resp));
}
