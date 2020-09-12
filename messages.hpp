#pragma once

#include <QAbstractListModel>

#include "core.grpc.pb.h"
#include "core.pb.h"

#include "client.hpp"
#include "util.hpp"

struct MessageData
{
	QString text;
	QString authorName;
	quint64 id;
	QDateTime date;

	static MessageData fromProtobuf(protocol::core::v1::Message& msg) {
		return MessageData {
			.text = QString::fromStdString(msg.content()),
			.authorName = QString::number(msg.author_id()),
			.id = msg.location().message_id(),
			.date = QDateTime::fromTime_t(msg.created_at().seconds())
		};
	}
};

typedef CarrierEvent<3,protocol::core::v1::GuildEvent_MessageSent> MessageSentEvent;

class ChannelsModel;

class MessagesModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	quint64 channelID;

	QList<MessageData> messageData;

	friend class ChannelsModel;
	friend class Client;

	bool atEnd = false;

	Client* client;

	enum Roles {
		MessageTextRole = Qt::UserRole,
		MessageAuthorRole,
		MessageDateRole
	};

protected:
	Q_INVOKABLE void customEvent(QEvent *event) override;

public:
	MessagesModel(QObject *parent, QString homeServer, quint64 guildID, quint64 channelID);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
	bool canFetchMore(const QModelIndex& parent) const override;
	void fetchMore(const QModelIndex& parent) override;

	Q_INVOKABLE void sendMessage(const QString &content);
};
