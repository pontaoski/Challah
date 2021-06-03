#pragma once

#include <QAbstractListModel>

#include "client.h"

#include "qivipendingreply.h"

class State;

class InviteModel : public QAbstractListModel
{
	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	SDK::Client* c;

public:
	InviteModel(SDK::Client* client, quint64 guildID, State* state);
	~InviteModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

	Q_INVOKABLE QIviPendingReply<bool> createInvite(const QString& id, qint32 possibleUses);
	Q_INVOKABLE QIviPendingReply<bool> deleteInvite(const QString& id);
};
