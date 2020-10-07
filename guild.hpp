// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QAbstractListModel>

class Client;

struct Guild {
	quint64 guildID;
	quint64 ownerID;
	QString homeserver;
	QString name;
	QString picture;
};

Q_DECLARE_METATYPE(Guild)

struct GuildRepl {
	quint64 ownerID;
	QString name;
	QString picture;
};

class GuildModel : public QAbstractListModel
{
	Q_OBJECT

	class Private;
	mutable Private* d;

	QList<Guild> guilds;
	friend class Client;

	Q_SIGNAL void addGuild(Guild data);
	Q_SLOT void addGuildHandler(Guild data);
	Q_SIGNAL void removeGuild(const QString& homeserver, quint64 id);
	Q_SLOT void removeGuildHandler(const QString& homeserver, quint64 id);

	enum Roles {
		GuildIDRole = Qt::UserRole,
		GuildNameRole,
		GuildPictureRole,
		ChannelModelRole,
		HomeserverRole,
		IsOwnerRole
	};

public:
	GuildModel();
	~GuildModel();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
};