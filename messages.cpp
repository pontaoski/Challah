// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

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
	permissions = new QQmlPropertyMap(this);

	permissions->insert("canSendAndEdit", client->hasPermission("messages.send", guildID));
	permissions->insert("canDeleteOthers", client->hasPermission("messages.manage.delete", guildID));

	{
		ClientContext ctx;
		client->authenticate(ctx);

		protocol::core::v1::GetGuildRequest req;
		req.set_guild_id(guildID);
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
			if (message.id == msg.message_id()) {
				message.editedAt = QDateTime::fromTime_t(msg.edited_at().seconds());

				std::string jsonified;
				google::protobuf::util::MessageToJsonString(msg, &jsonified, google::protobuf::util::JsonPrintOptions{});
				auto document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonified));

				if (msg.update_actions()) {
					message.actions = document["actions"];
				}
				if (msg.update_attachments()) {
					auto msgAttaches = msg.attachments();
					QStringList attachments;
					for (auto attach : msgAttaches) {
						attachments << QString::fromStdString(attach);
					}
					message.attachments = attachments;
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
		auto msg = ev->data.message_id();
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

	auto author = [=]() {
		if (messageData[idx].overrides.has_value()) {
			return messageData[idx].overrides->name;
		}
		return qobject_cast<ChannelsModel*>(parent())->userName(messageData[idx].authorID);
	};
	auto avatar = [=]() {
		if (messageData[idx].overrides.has_value()) {
			return messageData[idx].overrides->avatar;
		}
		return qobject_cast<ChannelsModel*>(parent())->avatarURL(messageData[idx].authorID);
	};
	auto isNextDifferent = [=]() {
		if (messageData.length() <= (idx+1))
			return true;

		if (messageData[idx].overrides.has_value() != messageData[idx+1].overrides.has_value())
			return true;

		if ((!messageData[idx].overrides.has_value()) == (!messageData[idx+1].overrides.has_value()))
			return false;

		const auto& lhs = messageData[idx].overrides.value();
		const auto& rhs = messageData[idx+1].overrides.value();

		if (lhs.avatar != rhs.avatar)
			return true;

		if (lhs.name != rhs.name)
			return true;

		return false;
	};

	switch (role)
	{
	case MessageTextRole:
		return messageData[idx].text;
	case MessageAuthorRole:
		return author();
	case MessageAuthorAvatarRole:
		return avatar();
	case MessageAuthorIDRole:
		return QString::number(messageData[idx].authorID);
	case MessageShouldDisplayAuthorInfo:
		if (messageData.length() <= (idx+1))
			return true;
		return (messageData[idx+1].authorID != messageData[idx].authorID) or isNextDifferent();
	case MessageDateRole:
		return messageData[idx].date;
	case MessageEmbedsRole:
		return messageData[idx].embeds;
	case MessageActionsRole:
		return messageData[idx].actions;
	case MessageIDRole:
		return QString::number(messageData[idx].id);
	case MessageReplyToRole:
		return messageData[idx].replyTo != 0 ? QString::number(messageData[idx].replyTo) : QVariant();
	case MessageAttachmentsRole:
		return messageData[idx].attachments;
	case MessageCombinedAuthorIDAvatarRole:
		return QStringList{
			QString::number(messageData[idx].authorID),
			avatar(),
			author()
		}.join("\t");
	}

	return QVariant();
}

QHash<int,QByteArray> MessagesModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[MessageTextRole] = "content";
	ret[MessageAuthorRole] = "authorName";
	ret[MessageAuthorAvatarRole] = "authorAvatar";
	ret[MessageAuthorIDRole] = "authorID";
	ret[MessageDateRole] = "date";
	ret[MessageEmbedsRole] = "embeds";
	ret[MessageActionsRole] = "actions";
	ret[MessageIDRole] = "messageID";
	ret[MessageShouldDisplayAuthorInfo] = "shouldShowAuthorInfo";
	ret[MessageReplyToRole] = "replyToID";
	ret[MessageCombinedAuthorIDAvatarRole] = "authorIDAndAvatar";
	ret[MessageAttachmentsRole] = "attachments";

	return ret;
}

void MessagesModel::sendMessageFull(const QString& message, const QString &replyTo, const QStringList& attachments, const SendAs& as)
{
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::SendMessageRequest req;

	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_content(message.toStdString());
	if (replyTo != QString()) {
		req.set_in_reply_to(replyTo.toULongLong());
	}

	for (auto attachment : attachments) {
		req.add_attachments(attachment.toStdString());
	}

	if (std::holds_alternative<Nobody>(as)) {

	} else if (std::holds_alternative<Fronter>(as)) {
		auto& fronter = std::get<Fronter>(as);

		auto override = new protocol::core::v1::Override();
		override->set_name(fronter.name.toStdString());
		override->set_allocated_system_plurality(new google::protobuf::Empty{});

		req.set_allocated_overrides(override);

	} else if (std::holds_alternative<RoleplayCharacter>(as)) {
		auto& character = std::get<RoleplayCharacter>(as);

		auto override = new protocol::core::v1::Override();
		override->set_name(character.name.toStdString());
		override->set_user_defined("Roleplay");

		req.set_allocated_overrides(override);
	}

	protocol::core::v1::SendMessageResponse empty;

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
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
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
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(messageID.toULongLong());
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
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(id.toULongLong());
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
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(id.toULongLong());
	google::protobuf::Empty resp;

	checkStatus(client->coreKit->DeleteMessage(&ctx, req, &resp));
}

QVariantMap MessagesModel::peekMessage(const QString& id)
{
	quint64 actualID = id.toULongLong();
	for (const auto& item : messageData) {
		if (item.id == actualID) {
			return {
				{ "authorName", qobject_cast<ChannelsModel*>(parent())->userName(item.authorID) },
				{ "content", item.text }
			};
		}
	}

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::GetMessageRequest req;
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(actualID);
	protocol::core::v1::GetMessageResponse resp;

	if (!checkStatus(client->coreKit->GetMessage(&ctx, req, &resp))) {
		return QVariantMap();
	}

	return {
		{ "authorName", qobject_cast<ChannelsModel*>(parent())->userName(resp.message().author_id()) },
		{ "content", QString::fromStdString(resp.message().content()) }
	};
}
