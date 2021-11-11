#pragma once

#include "qabstractrelationalmodel.h"
#include <QAbstractListModel>

#include "client.h"

class State;
class ChannelsStore;

class ChannelsModel : public QAbstractListModel
{

	Q_OBJECT
	Q_PROPERTY(ChannelsStore* store READ store CONSTANT)
	Q_PROPERTY(bool working READ working NOTIFY workingChanged)

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	QString host;

	friend class ChannelsStore;

public:
	ChannelsModel(QString host, quint64 guildID, State* state);
	~ChannelsModel();

	ChannelsStore* store();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;

	bool working() const;
	Q_SIGNAL void workingChanged();

	Q_INVOKABLE void newChannel(const QString& name, const QString& type = "");
	Q_INVOKABLE void moveChannel(const QString& id, int idx);

};

class ChannelsStore : public ChallahAbstractRelationalModel
{

	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;

	friend class ChannelsModel;

public:
	ChannelsStore(State* state, ChannelsModel* parent);
	~ChannelsStore();

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	bool checkKey(const QVariant& key) override;
	QHash<int,QByteArray> roleNames() override;

	Q_INVOKABLE void setChannelName(const QString& id, const QString& name);

};
