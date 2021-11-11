#include <QQmlEngine>

#include "state.h"
#include "voice.h"

voiceCall*
State::makeVoiceCall(QString host, QString guildID, QString channelID, QObject* it)
{
	auto vc = new voiceCall(this, host, guildID.toULongLong(), channelID.toULongLong());
	qmlEngine(it)->setObjectOwnership(vc, QQmlEngine::JavaScriptOwnership);

	return vc;
}

