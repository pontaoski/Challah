#pragma once

#include <QVariant>
#include <memory>
#include "qabstractrelationalmodel.h"

#include "state.h"

class OwnPermissionsStore : public ChallahAbstractRelationalModel
{

	Q_OBJECT

	State* state;

	struct Private;
	std::unique_ptr<Private> d;

public:

	explicit OwnPermissionsStore(State* state);
	~OwnPermissionsStore();

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	bool checkKey(const QVariant& key) override;
	bool canFetchKey(const QVariant& key) override;
	void fetchKey(const QVariant& key) override;

	QHash<int, QByteArray> roleNames() override;

};
