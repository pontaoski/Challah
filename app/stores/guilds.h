#pragma once

#include <QAbstractListModel>
#include "relationallib/qabstractrelationalmodel.h"
#include "state.h"

class GuildsStore : public ChallahAbstractRelationalModel
{
	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;
	State* s;

public:

	GuildsStore(State* parent);
	~GuildsStore();

	bool checkKey(const QVariant& key) override;
	bool canFetchKey(const QVariant& key) override;
	void fetchKey(const QVariant& key) override;

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	QHash<int, QByteArray> roleNames() override;

	Q_INVOKABLE void setName(const QString& host, const QString& guildID, const QString& name);
	Q_INVOKABLE void setPicture(const QString& host, const QString& guildID, const QUrl& photo);

};

class GuildList : public QAbstractListModel
{

	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;
	State* s;

public:

	GuildList(State* parent);
	~GuildList();

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

	Q_INVOKABLE void leave(const QString& host, const QString& guildID);

};
