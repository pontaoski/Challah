#pragma once

#include <QScopedPointer>

#include <voice/v1/voice.hrpc.types.h>

#include "state.h"

namespace voice = protocol::voice::v1;
using GstPromise = struct _GstPromise;

class voiceCall : public QObject
{

	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;
	State* s;

protected:
	void initialised(QString rtpCapabilities);
	void preparedForJoinChannel(voice::TransportOptions consumer, voice::TransportOptions producer);
	void joinedChannel(QList<voice::UserConsumerOptions> otherUsers);
	void userJoined(voice::UserConsumerOptions userData);
	void userLeft(quint64 user);
	void offerCreated(GstPromise* promise);
	void remoteDescriptionCreated(GstPromise* promise);
	void startSignalling();
	bool createPipeline(int opusPayloadType);

	friend void offerCreated_(GstPromise *promise, void* self);
	friend void remoteDescriptionCreated_(GstPromise *promise, void* self);

public:
	voiceCall(State* state, const QString& host, quint64 guildID, quint64 channelID);
	~voiceCall();

};
