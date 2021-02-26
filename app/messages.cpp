// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtConcurrent>
#include <google/protobuf/empty.pb.h>

#include "channels.hpp"
#include "chat/v1/channels.pb.h"
#include "messages.hpp"
#include "qcoreapplication.h"
#include "util.hpp"
#include "state.hpp"

#define theHeaders {{"Authorization", client->userToken}}

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

	QtConcurrent::run([=] {
		protocol::chat::v1::GetGuildRequest req;
		req.set_guild_id(guildID);

		auto result = client->chatKit->GetGuild(req, theHeaders);
		if (resultOk(result)) {
			isGuildOwner = client->userID == unwrap(result).guild_owner();
		}
	});
}

void MessagesModel::customEvent(QEvent *event)
{
	if (event->type() == MessageSentEvent::typeID) {
		auto ev = reinterpret_cast<MessageSentEvent*>(event);
		auto echoID = ev->data.echo_id();

		userCounts.remove(ev->data.message().author_id());
		typingIndicatorChanged();

		if (echoes.contains(echoID)) {
			auto msg = echoes[echoID];
			int i = 0;
			for (auto &message : messageData) {
				if (message.echoID == echoID) {
					break;
				}
				i++;
			}
			auto message = ev->data.message();
			*msg = MessageData::fromProtobuf(message);

			echoes.remove(echoID);

			dataChanged(index(i), index(i));
		} else {
			auto msg = ev->data.message();
			auto idx = messageData.count();
			beginInsertRows(QModelIndex(), 0, 0);
			messageData.push_front(MessageData::fromProtobuf(msg));
			endInsertRows();
		}

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
					QVariantList attachments;
					for (auto attach : msgAttaches) {
						std::string jsonified;
						google::protobuf::util::MessageToJsonString(attach, &jsonified, google::protobuf::util::JsonPrintOptions{});
						auto document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonified));

						attachments << document.object();
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
	} else if (event->type() == TypingEvent::typeID) {
		auto ev = reinterpret_cast<TypingEvent*>(event);
		auto& data = ev->data;
		auto uid = data.user_id();

		if (uid == client->userID) {
			return;
		}

		this->userCounts[uid] = this->userCounts.value(uid, 0) + 1;
		typingIndicatorChanged();

		QTimer::singleShot(3000, [this, uid] {
			if (this->userCounts.contains(uid)) {
				this->userCounts[uid]--;
				if (this->userCounts[uid] < 1) {
					this->userCounts.remove(uid);
				}
			}
			typingIndicatorChanged();
		});
	} else if (event->type() == FetchMessagesEvent::typeID) {
		auto resp = reinterpret_cast<FetchMessagesEvent*>(event)->data;

		if (resp.messages_size() == 0) {
			atEnd = true;
			return;
		}

		beginInsertRows(QModelIndex(), messageData.count(), (messageData.count()+resp.messages_size())-1);
		for (auto item : resp.messages()) {
			messageData << MessageData::fromProtobuf(item);
		}
		endInsertRows();

		isReadingMore = false;
	}
}

QString localiseList(const QStringList& list)
{
	auto copy = list;

	switch (copy.length()) {
	case 0:
		return QString();
	case 1:
		return copy[0];
	case 2:
		return QCoreApplication::tr("%1 and %2", "Lists").arg(copy[0]).arg(copy[1]);
	}

	Q_ASSERT(copy.length() >= 3);

	auto front = copy.front();
	copy.pop_front();

	auto back = copy.back();
	copy.pop_back();

	auto middle = copy.front();
	copy.pop_front();

	while (copy.length() > 0) {
		auto head = copy.front();
		copy.pop_front();

		//: the middle of a list with 4 or more items: "A, %1, %2, D, and E"
		middle = QCoreApplication::tr("%1, %2", "Lists").arg(middle).arg(head);
	}

	//: the start of a list, for example "%1, %2, and C"
	auto start = QCoreApplication::tr("%1, %2", "Lists").arg(front).arg(middle);

	//: the end of a list, for example "A, %1, and %2"
	auto end = QCoreApplication::tr("%1, and %2", "Lists").arg(start).arg(back);

	Q_ASSERT(copy.length() == 0);

	return end;
}

QString MessagesModel::typingIndicator()
{
	if (userCounts.keys().length() == 0) {
		return QString();
	}

	QStringList names;
	for (auto user : userCounts.keys()) {
		names << channelsModel()->userName(user);
	}
	names.sort();

	return tr("%1 are typing...", "Lists", names.count()).arg(localiseList(names));
}

void MessagesModel::typed()
{
	static bool shouldSend = true;
	if (!shouldSend) return;

	shouldSend = false;

	QTimer::singleShot(1000, [] {
		shouldSend = true;
	});

	QtConcurrent::run([=]{
		protocol::chat::v1::TypingRequest req;
		req.set_guild_id(guildID);
		req.set_channel_id(channelID);

		resultOk(client->chatKit->Typing(req, theHeaders));
	});
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

	auto author = [this](int idx) {
		if (messageData[idx].overrides.has_value()) {
			return messageData[idx].overrides->name;
		}
		return qobject_cast<ChannelsModel*>(parent())->userName(messageData[idx].authorID);
	};
	auto avatar = [this](int idx) {
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

		const auto& lhs = *messageData[idx].overrides;
		const auto& rhs = *messageData[idx+1].overrides;

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
		return author(idx);
	case MessageAuthorAvatarRole:
		return avatar(idx);
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
			avatar(idx),
			author(idx),
			messageData[idx].date.date().toString(),
		}.join("\t");
	case IsOwnMessageRole:
		return messageData[idx].authorID == client->userID;
	case MessageQuirkRole: {
		QVariantMap quirks;
		if (messageData.length() > (idx+1)) {

			auto thisData = messageData[idx].date;
			auto thatData = messageData[idx+1].date;
			if (thisData.date() != thatData.date()) {
				quirks["dateHeader"] = QLocale::system().toString(thisData.date(), QLocale::LongFormat);
			}

			quirks["previousAuthorDifferent"] = (messageData[idx].authorID != messageData[idx+1].authorID) || (avatar(idx) != avatar(idx+1)) || (author(idx) != author(idx+1));
		}

		return quirks;
	}
	case MessageModelIndexRole:
		return QVariant::fromValue(index);
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
	ret[MessageQuirkRole] = "quirks";
	ret[MessageModelIndexRole] = "modelIndex";
	ret[IsOwnMessageRole] = "isOwnMessage";

	return ret;
}

void MessagesModel::sendMessageFull(const QString& message, const QString &replyTo, const QStringList& attachments, const SendAs& as)
{
	protocol::chat::v1::SendMessageRequest req;

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

	} else if (auto fronter = std::get_if<Fronter>(&as)) {
		auto override = new protocol::harmonytypes::v1::Override();
		override->set_name(fronter->name.toStdString());
		override->set_allocated_system_plurality(new google::protobuf::Empty{});

		req.set_allocated_overrides(override);

	} else if (auto character = std::get_if<RoleplayCharacter>(&as)) {
		auto override = new protocol::harmonytypes::v1::Override();
		override->set_name(character->name.toStdString());
		override->set_user_defined("Roleplay");

		req.set_allocated_overrides(override);
	}

	beginInsertRows(QModelIndex(), 0, 0);
	auto incoming = MessageData {};
	incoming.text = message;
	if (replyTo != QString()) {
		incoming.replyTo = replyTo.toULongLong();
	}
	QVariantList attaches;
	for (auto attach : attachments) {
		attaches << attach;
	}
	incoming.attachments = attaches;
	if (std::holds_alternative<Nobody>(as)) {
		;
	} else if (auto fronter = std::get_if<Fronter>(&as)) {
		incoming.overrides = MessageData::Override {
			fronter->name,
			QString(),
			MessageData::Override::Plurality,
		};
	} else if (auto character = std::get_if<RoleplayCharacter>(&as)) {
		incoming.overrides = MessageData::Override {
			character->name,
			QString(),
			MessageData::Override::Reason(0),
		};
	}
	incoming.status = MessageData::Sending;
	incoming.authorID = client->userID;

	auto echoID = QRandomGenerator::global()->generate64();
	req.set_echo_id(echoID);
	incoming.echoID = echoID;

	messageData.push_front(incoming);
	endInsertRows();

	MessageData& ref = messageData[0];
	echoes[echoID] = &ref;

	protocol::chat::v1::SendMessageResponse empty;

	QtConcurrent::run([=]() mutable {
		client->chatKit->SendMessage(req, theHeaders);
	});
}

bool MessagesModel::canFetchMore(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return !atEnd;
}

void MessagesModel::fetchMore(const QModelIndex& parent)
{
	Q_UNUSED(parent)

	if (isReadingMore) {
		return;
	}

	isReadingMore = true;

	protocol::chat::v1::GetChannelMessagesRequest req;
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	if (!messageData.isEmpty()) {
		req.set_before_message(messageData.last().id);
	}

	QtConcurrent::run([=] {
		auto result = client->chatKit->GetChannelMessages(req, theHeaders);
		if (resultOk(result)) {
			auto resp = unwrap(result);
			qDebug() << "posting got more messages";
			QCoreApplication::postEvent(this, new FetchMessagesEvent(resp));
		} else {
			runOnMainThread("failed to get more messages", [this] {
				isReadingMore = false;
			});
		}
	});
}

void MessagesModel::triggerAction(const QString& messageID, const QString &name, const QString &data)
{
	protocol::chat::v1::TriggerActionRequest req;
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(messageID.toULongLong());
	req.set_action_id(name.toStdString());
	if (data != QString()) {
		req.set_action_data(data.toStdString());
	}

	client->chatKit->TriggerAction(req, theHeaders);
}

void MessagesModel::editMessage(const QString& id, const QString &content)
{
	protocol::chat::v1::UpdateMessageRequest req;
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(id.toULongLong());
	req.set_content(content.toStdString());
	req.set_update_content(true);

	client->chatKit->UpdateMessage(req, theHeaders);
}

void MessagesModel::deleteMessage(const QString& id)
{
	protocol::chat::v1::DeleteMessageRequest req;
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(id.toULongLong());

	client->chatKit->DeleteMessage(req, theHeaders);
}

QVariantMap MessagesModel::peekMessage(const QString& id)
{
	quint64 actualID = id.toULongLong();

	auto author = [this](const MessageData& it) {
		if (it.overrides.has_value()) {
			return it.overrides->name;
		}
		return qobject_cast<ChannelsModel*>(parent())->userName(it.authorID);
	};

	for (const auto& item : messageData) {
		if (item.id == actualID) {
			return {
				{ "authorName", author(item) },
				{ "content", item.text }
			};
		}
	}

	protocol::chat::v1::GetMessageRequest req;
	req.set_guild_id(guildID);
	req.set_channel_id(channelID);
	req.set_message_id(actualID);
	protocol::chat::v1::GetMessageResponse resp;

	auto result = client->chatKit->GetMessage(req, theHeaders);
	if (!resultOk(result)) {
		return QVariantMap();
	}

	auto name = QString();
	if (resp.message().has_overrides()) {
		name = QString::fromStdString(resp.message().overrides().name());
	} else {
		name = qobject_cast<ChannelsModel*>(parent())->userName(resp.message().author_id());
	}

	return {
		{ "authorName", name },
		{ "content", QString::fromStdString(resp.message().content()) }
	};
}
