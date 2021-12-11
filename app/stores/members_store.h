#pragma once

#include "qabstractrelationalmodel.h"

#include "client.h"

class State;

class MembersStore : public ChallahAbstractRelationalModel
{

	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;

	void fetchBatched();

public:
	MembersStore(State* state);
	~MembersStore();

	bool checkKey(const QVariant& key) override;
	bool canFetchKey(const QVariant& key) override;
	void fetchKey(const QVariant& key) override;

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() override;

};
