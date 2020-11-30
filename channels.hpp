// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlPropertyMap>

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

class ChannelsModel;
class InviteModel;
class RolesModel;
class MembersModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	QVector<quint64> members;
	QString _name;
	QString _picture;

	friend class Client;
	friend class ChannelsModel;
	Client* client;
	ChannelsModel* model;

	enum Roles {
		MemberNameRole = Qt::DisplayRole,
		MemberAvatarRole = Qt::DecorationRole,
	};

	Q_PROPERTY(QString name READ name NOTIFY nameChanged)
	Q_PROPERTY(QString picture READ picture NOTIFY pictureChanged)

protected:
	void customEvent(QEvent *event) override {
		if (event->type() == MemberJoinedEvent::typeID) {
			auto ev = reinterpret_cast<MemberJoinedEvent*>(event);
			beginInsertRows(QModelIndex(), members.length(), members.length());
			members << ev->data.member_id();
			endInsertRows();
		} else if (event->type() == MemberLeftEvent::typeID) {
			auto ev = reinterpret_cast<MemberLeftEvent*>(event);
			auto idx = std::find_if(members.begin(), members.end(), [=](quint64 id) { return id == ev->data.member_id(); });

			beginRemoveRows(QModelIndex(), idx - members.begin(), idx - members.begin());
			members.removeAt(idx - members.begin());
			endRemoveRows();
		} else if (event->type() == GuildUpdatedEvent::typeID) {
			auto ev = reinterpret_cast<GuildUpdatedEvent*>(event);
			if (ev->data.update_name()) {
				_name = QString::fromStdString(ev->data.name());
				Q_EMIT nameChanged();
			}
		}
	}

public:
	Q_SIGNAL void nameChanged();
	Q_SIGNAL void pictureChanged();
	QString name() const { return _name; }
	QString picture() const { return _picture; }

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
	static QMap<QPair<QString,quint64>,ChannelsModel*> instances;
	QQmlPropertyMap* permissions;

	Client* client;
	MembersModel* members;

	enum Roles {
		ChannelIDRole = Qt::UserRole,
		ChannelNameRole,
		ChannelIsCategoryRole,
		MessageModelRole
	};

	Q_PROPERTY(MembersModel* members READ getMembers CONSTANT FINAL)
	Q_PROPERTY(QQmlPropertyMap* permissions MEMBER permissions CONSTANT FINAL)

protected:
	Q_INVOKABLE void customEvent(QEvent *event) override;

public:
	ChannelsModel(QString homeServer, quint64 guildID);
	~ChannelsModel() { instances.remove(qMakePair(homeServer, guildID)); }
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
	MembersModel* getMembers() const { return members; }
	static ChannelsModel* modelFor(QString& homeserver, quint64 guild) {
		return instances.value(qMakePair(homeserver, guild));
	}

	Q_INVOKABLE void deleteChannel(const QString& id);
	Q_INVOKABLE bool createChannel(const QString& name);
	Q_INVOKABLE void moveChannelFromTo(int from, int to);
	Q_INVOKABLE QString userName(quint64 id);
	Q_INVOKABLE QString avatarURL(quint64 id);
	Q_INVOKABLE InviteModel* invitesModel();
	Q_INVOKABLE RolesModel* rolesModel();
};
