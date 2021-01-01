#pragma once

#include <QAbstractListModel>
#include "channels.hpp"

struct Invite {
	QString id;
	qint32 possibleUses;
	qint32 useCount;
};

class InviteModel : public QAbstractListModel
{
	Q_OBJECT

	QList<Invite> invites;
	QString homeserver;
	quint64 guildID;
	Client* client;

public:
	InviteModel(ChannelsModel *parent, QString homeServer, quint64 guildID);

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

	enum Roles {
		ID = Qt::UserRole,
		PossibleUses,
		Uses,
	};

	Q_INVOKABLE bool createInvite(const QString& id, qint32 possibleUses);
	Q_INVOKABLE bool deleteInvite(const QString& id);
};
