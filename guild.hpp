#pragma once

#include <QAbstractListModel>

class Client;

struct Guild {
	quint64 guildID;
	QString homeserver;
	QString name;
	QString picture;
};

Q_DECLARE_METATYPE(Guild)

struct GuildRepl {
	QString name;
	QString picture;
};

class GuildModel : public QAbstractListModel
{
	Q_OBJECT

	QList<Guild> guilds;
	friend class Client;

	Q_SIGNAL void addGuild(Guild data);
	Q_SLOT void addGuildHandler(Guild data);

	enum Roles {
		GuildIDRole = Qt::UserRole,
		GuildNameRole,
		GuildPictureRole,
		ChannelModelRole
	};

public:
	GuildModel();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
};