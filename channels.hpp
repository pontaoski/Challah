#pragma once

#include <QAbstractListModel>

class Client;

struct Channel {
	quint64 channelID;
	QString name;
	bool isCategory;
};

class ChannelsModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;
	QList<Channel> channels;
	friend class Client;

	Client* client;

	enum Roles {
		ChannelIDRole = Qt::UserRole,
		ChannelNameRole,
		ChannelIsCategoryRole
	};

public:
	ChannelsModel(QString homeServer, quint64 guildID);
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
};