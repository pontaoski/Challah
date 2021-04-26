#pragma once

#include <QAbstractListModel>

#include "client.h"

class State;
class MessagesStore;

class MessagesModel : public QAbstractListModel
{

	Q_OBJECT
	Q_PROPERTY(MessagesStore* store READ store CONSTANT)

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	SDK::Client* c;

public:
	MessagesModel(SDK::Client* client, quint64 guildID, quint64 channelID, State* state);
	~MessagesModel();

	MessagesStore* store();

	Q_INVOKABLE void send(const QString& txt);

	bool canFetchMore(const QModelIndex &parent) const override;
	void fetchMore(const QModelIndex &parent) override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

};
