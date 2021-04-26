#pragma once

#include <QVariant>
#include <memory>
#include "qabstractrelationalmodel.h"

#include "client.h"

class MessagesModel;

class MessagesStore : public ChallahAbstractRelationalModel
{

	Q_OBJECT

	SDK::Client* c;
	MessagesModel* p;

	struct Private;
	std::unique_ptr<Private> d;

public:

	explicit MessagesStore(MessagesModel* parent, SDK::Client* client);
	~MessagesStore();

	void newMessage(quint64 id, protocol::harmonytypes::v1::Message cont);

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	bool checkKey(const QVariant& key) override;
	bool canFetchKey(const QVariant& key) override;
	void fetchKey(const QVariant& key) override;

	QHash<int, QByteArray> roleNames() override;

};
