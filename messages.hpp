// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QJsonDocument>
#include <QJSValue>
#include <optional>

#include <google/protobuf/util/json_util.h>

#include "core.grpc.pb.h"
#include "core.pb.h"

#include "client.hpp"
#include "util.hpp"

struct MessageData
{
	QString text;
	quint64 authorID;
	quint64 id;
	QDateTime date;
	QJsonValue actions;
	QJsonValue embeds;
	QDateTime editedAt;
	quint64 replyTo;

	struct Override
	{
		QString name;
		QString avatar;

		enum Reason {
			Plurality,
			Bridge,
			Webhook
		};
		Reason reason;
	};
	std::optional<Override> overrides;

	QStringList attachments;

	static MessageData fromProtobuf(protocol::core::v1::Message& msg) {
		std::string jsonified;
		google::protobuf::util::MessageToJsonString(msg, &jsonified, google::protobuf::util::JsonPrintOptions{});
		auto document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonified));

		std::optional<Override> overrides;
		if (msg.has_overrides()) {
			overrides = Override{};
			overrides->name = QString::fromStdString(msg.overrides().name());
			overrides->avatar = QString::fromStdString(msg.overrides().avatar());
			if (msg.overrides().has_system_plurality()) {
				overrides->reason = Override::Plurality;
			} else if (msg.overrides().has_webhook()) {
				overrides->reason = Override::Webhook;
			} else if (msg.overrides().has_bridge()) {
				overrides->reason = Override::Bridge;
			}
		}

		auto msgAttaches = msg.attachments();
		QStringList attachments;
		for (auto attach : msgAttaches) {
			attachments << QString::fromStdString(attach);
		}

		return MessageData {
			.text = QString::fromStdString(msg.content()),
			.authorID = msg.author_id(),
			.id = msg.message_id(),
			.date = QDateTime::fromTime_t(msg.created_at().seconds()),
			.actions = document["actions"],
			.embeds = document["embeds"],
			.editedAt = QDateTime(),
			.replyTo = msg.in_reply_to(),
			.overrides = overrides,
			.attachments = attachments
		};
	}
};

class ChannelsModel;
class QNetworkAccessManager;

class MessagesModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	quint64 channelID;

	QList<MessageData> messageData;
	QSharedPointer<QNetworkAccessManager> nam;

	friend class ChannelsModel;
	friend class Client;

	bool atEnd = false;
	bool isGuildOwner = false;

	Client* client;

	enum Roles {
		MessageTextRole = Qt::UserRole,
		MessageEmbedsRole,
		MessageActionsRole,
		MessageAuthorRole,
		MessageAuthorAvatarRole,
		MessageAuthorIDRole,
		MessageShouldDisplayAuthorInfo,
		MessageDateRole,
		MessageReplyToRole,
		MessageIDRole,
		MessageAttachmentsRole,
		MessageCombinedAuthorIDAvatarRole
	};

	struct Fronter {
		QString name;
	};
	struct RoleplayCharacter {
		QString name;
	};

	using Nobody = std::monostate;
	using SendAs = std::variant<Nobody, Fronter, RoleplayCharacter>;

protected:
	Q_INVOKABLE void customEvent(QEvent *event) override;

public:
	MessagesModel(ChannelsModel *parent, QString homeServer, quint64 guildID, quint64 channelID);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
	bool canFetchMore(const QModelIndex& parent) const override;
	void fetchMore(const QModelIndex& parent) override;

	Q_INVOKABLE bool isOwner() { return isGuildOwner; }
	Q_INVOKABLE QString userID() { return QString::number(client->userID); }
	Q_INVOKABLE QVariantMap peekMessage(const QString& id);
	Q_INVOKABLE void sendMessageFull(const QString& content, const QString& replyTo, const QStringList& attachments, const SendAs& as);
	Q_INVOKABLE void sendMessage(const QString& content, const QString& replyTo, const QStringList& attachments)
	{
		sendMessageFull(content, replyTo, attachments, SendAs(Nobody{}));
	}
	Q_INVOKABLE void sendMessageAsSystem(const QString& content, const QString& replyTo, const QStringList& attachments, const QString& memberName)
	{
		sendMessageFull(content, replyTo, attachments, SendAs(Fronter {
			.name = memberName
		}));
	}
	Q_INVOKABLE void sendMessageAsRoleplay(const QString& content, const QString& replyTo, const QStringList& attachments, const QString& characterName)
	{
		sendMessageFull(content, replyTo, attachments, SendAs(RoleplayCharacter {
			.name = characterName
		}));
	}
	Q_INVOKABLE void editMessage(const QString& id, const QString& content);
	Q_INVOKABLE void deleteMessage(const QString& id);
	Q_INVOKABLE void triggerAction(const QString& messageID, const QString& name, const QString& data);
	Q_INVOKABLE void uploadFile(const QUrl& path, QJSValue then, QJSValue elseDo, QJSValue progress, QJSValue finally);
};
