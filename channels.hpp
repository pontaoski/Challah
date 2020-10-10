// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

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
typedef CarrierEvent<7,protocol::core::v1::GuildEvent_MemberLeft> MemberLeftEvent;

class ChannelsModel;
class InviteModel;
class MembersModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	QVector<quint64> members;

	friend class Client;
	Client* client;
	ChannelsModel* model;

	enum Roles {
		MemberNameRole = Qt::DisplayRole,
		MemberAvatarRole = Qt::DecorationRole,
	};

protected:
	void customEvent(QEvent *event) override {
		if (event->type() == MemberJoinEvent::typeID) {
			auto ev = reinterpret_cast<MemberJoinEvent*>(event);
			beginInsertRows(QModelIndex(), members.length(), members.length());
			members << ev->data.member_id();
			endInsertRows();
		} else if (event->type() == MemberLeftEvent::typeID) {
			auto ev = reinterpret_cast<MemberLeftEvent*>(event);
			auto idx = std::find_if(members.begin(), members.end(), [=](quint64 id) { return id == ev->data.member_id(); });

			beginRemoveRows(QModelIndex(), idx - members.begin(), idx - members.begin());
			members.removeAt(idx - members.begin());
			endRemoveRows();
		}
	}

public:
	MembersModel(QString homeServer, quint64 guildID, ChannelsModel* model);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

class ChannelsModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	QList<Channel> channels;
	QMap<quint64,QString> users;
	QMap<quint64,QString> avatars;
	mutable QMap<quint64,MessagesModel*> models;
	friend class Client;

	Client* client;
	MembersModel* members;

	enum Roles {
		ChannelIDRole = Qt::UserRole,
		ChannelNameRole,
		ChannelIsCategoryRole,
		MessageModelRole
	};

	Q_PROPERTY(MembersModel* members READ getMembers CONSTANT FINAL)

protected:
	Q_INVOKABLE void customEvent(QEvent *event) override;

public:
	ChannelsModel(QString homeServer, quint64 guildID);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
	MembersModel* getMembers() const { return members; }

	Q_INVOKABLE void deleteChannel(const QString& id);
	Q_INVOKABLE bool createChannel(const QString& name);
	Q_INVOKABLE QString userName(quint64 id);
	Q_INVOKABLE QString avatarURL(quint64 id);
	Q_INVOKABLE InviteModel* invitesModel();
};
