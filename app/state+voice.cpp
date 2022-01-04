#include <QQmlEngine>

#include "state.h"

#if CHALLAH_ENABLE_VOICE
#include "voice.h"
#endif

voiceCall*
State::makeVoiceCall(QString host, QString guildID, QString channelID, QObject* it)
{
#if CHALLAH_ENABLE_VOICE
	auto vc = new voiceCall(this, host, guildID.toULongLong(), channelID.toULongLong());
	qmlEngine(it)->setObjectOwnership(vc, QQmlEngine::JavaScriptOwnership);

	return vc;
#else
	Q_UNUSED(host)
	Q_UNUSED(guildID)
	Q_UNUSED(channelID)
	Q_UNUSED(it)

	return nullptr;
#endif
}

