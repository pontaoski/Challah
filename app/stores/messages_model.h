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
	QString host;

public:
	MessagesModel(QString host, quint64 guildID, quint64 channelID, State* state);
	~MessagesModel();

	MessagesStore* store();

	Q_INVOKABLE Croutons::FutureBase send(QString txt, QString inReplyTo = "");
	Q_INVOKABLE Croutons::FutureBase sendFiles(const QList<QUrl>& txt);
	Q_INVOKABLE Croutons::FutureBase deleteMessage(const QString& id);

	bool canFetchMore(const QModelIndex &parent) const override;
	void fetchMore(const QModelIndex &parent) override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

};
