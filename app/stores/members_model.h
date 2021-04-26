#pragma once

#include <QAbstractListModel>

#include "client.h"

class State;

class MembersModel : public QAbstractListModel
{

	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	SDK::Client* c;

public:
	MembersModel(SDK::Client* client, quint64 guildID, State* state);
	~MembersModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

};
