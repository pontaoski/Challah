#pragma once

#include <QVariant>
#include <memory>
#include "qabstractrelationalmodel.h"

#include "client.h"

class MessagesModel;
class State;

struct MessageID {
	enum Kind {
		Remote,
		Echo,
		Invalid,
	};

	Kind kind;
	quint64 id;

	bool operator<(const MessageID& rhs) const {
		return kind < rhs.kind || id < rhs.id;
	}
	bool operator==(const MessageID& rhs) const {
		return kind == rhs.kind && id == rhs.id;
	}

	static MessageID fromString(const QString& str) {
		if (str.startsWith("echo:")) {
			return MessageID { Echo, str.mid(5).toULongLong() };
		} else if (str.startsWith("remote:")) {
			return MessageID { Remote, str.mid(7).toULongLong() };
		}
		return MessageID { Invalid, 0 };
	}
	QString toString() const {
		switch (kind) {
		case Echo:
			return "echo:" + QString::number(id);
		case Remote:
			return "remote:" + QString::number(id);
		case Invalid:
		default:
			return "invalid";
		}
	}
};

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
	void echoMessage(quint64 id, const protocol::chat::v1::SendMessageRequest& smr);
	void deleteMessage(quint64 id);
	void editMessage(quint64 id, protocol::chat::v1::FormattedText content);

	QVariant data(const QVariant& key, int role = Qt::DisplayRole) override;
	bool checkKey(const QVariant& key) override;
	bool canFetchKey(const QVariant& key) override;
	void fetchKey(const QVariant& key) override;

	QHash<int, QByteArray> roleNames() override;

};
