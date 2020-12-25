// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlPropertyMap>

#include "chat/v1/chat.grpc.pb.h"
#include "chat/v1/chat.pb.h"

#include "messages.hpp"
#include "util.hpp"

class Client;
class QNetworkAccessManager;

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

	Q_PROPERTY(ChannelsModel* parentModel READ channelsModel CONSTANT FINAL)
	ChannelsModel* channelsModel() { return model; }

protected:
	void customEvent(QEvent *event) override;

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
	QSharedPointer<QNetworkAccessManager> nam;
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
	Q_INVOKABLE void createChannel(const QString& name, QJSValue then, QJSValue elseDo);
	Q_INVOKABLE void moveChannelFromTo(int from, int to);
	Q_INVOKABLE QString userName(quint64 id);
	Q_INVOKABLE QString avatarURL(quint64 id);
	Q_INVOKABLE void setGuildPicture(const QString &url);
	Q_INVOKABLE void uploadFile(const QUrl& path, QJSValue then, QJSValue elseDo, QJSValue progress, QJSValue finally);
	Q_INVOKABLE InviteModel* invitesModel();
	Q_INVOKABLE RolesModel* rolesModel();
};
