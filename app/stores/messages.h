#pragma once

#include <QVariant>
#include <memory>
#include "qabstractrelationalmodel.h"

#include "client.h"

class MessagesModel;
class State;

class MessagesStore : public ChallahAbstractRelationalModel
{

	Q_OBJECT

	QString host;
	State* s;
	MessagesModel* p;

	struct Private;
	std::unique_ptr<Private> d;

public:

	explicit MessagesStore(MessagesModel* parent, State* state, QString host);
	~MessagesStore();

	void newMessage(quint64 id, protocol::chat::v1::Message cont);
	void deleteMessage(quint64 id);
	void editMessage(quint64 id, protocol::chat::v1::FormattedText content);

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	bool checkKey(const QVariant& key) override;
	bool canFetchKey(const QVariant& key) override;
	void fetchKey(const QVariant& key) override;

	QHash<int, QByteArray> roleNames() override;

};
