#pragma once

#include <QAbstractListModel>

#include "core.grpc.pb.h"
#include "core.pb.h"

#include "messages.hpp"
#include "util.hpp"

class Client;

struct Channel {
	quint64 channelID;
	QString name;
	bool isCategory;
};

typedef CarrierEvent<1,protocol::core::v1::GuildEvent_ChannelCreated> ChannelAddEvent;

typedef CarrierEvent<2,protocol::core::v1::GuildEvent_ChannelDeleted> ChannelDeleteEvent;
typedef CarrierEvent<6,protocol::core::v1::GuildEvent_MemberJoined> MemberJoinEvent;
typedef CarrierEvent<6,protocol::core::v1::GuildEvent_MemberLeft> MemberLeftEvent;

class ChannelsModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	QList<Channel> channels;
	QMap<quint64,QString> users;
	mutable QMap<quint64,MessagesModel*> models;
	friend class Client;

	Client* client;

	enum Roles {
		ChannelIDRole = Qt::UserRole,
		ChannelNameRole,
		ChannelIsCategoryRole,
		MessageModelRole
	};

protected:
	Q_INVOKABLE void customEvent(QEvent *event) override;

public:
	ChannelsModel(QString homeServer, quint64 guildID);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

	Q_INVOKABLE void deleteChannel(quint64 id);
	Q_INVOKABLE bool createChannel(const QString& name);
	Q_INVOKABLE QString userName(quint64 id);
};