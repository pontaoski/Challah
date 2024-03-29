#include "copyinterceptor.h"
#include "overlappingpanels.h"
#include "conditional.h"
#include "utils.h"

#include <QQuickStyle>
#include "qquickrelationallistener.h"
#include "setup.h"
#undef signal
#include "state.h"
#include "guilds.h"
#include "channels.h"
#include "members_model.h"
#include "members_store.h"
#include "messages_model.h"
#include "own_permissions.h"
#include "messages.h"
#include "invites.h"
#include "roles.h"

#if CHALLAH_ENABLE_VOICE
#include "voice.h"
#include <gst/gst.h>
#endif

#include "overrides_model.h"

#ifdef CHALLAH_VENDORED_KIRIGAMI

#define KIRIGAMI_BUILD_TYPE_STATIC

#include "../vendor/kirigami/src/kirigamiplugin.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(KirigamiPlugin)

#undef KIRIGAMI_BUILD_TYPE_STATIC

#endif

//

#ifdef CHALLAH_VENDORED_KITEMMODELS

#include "../vendor/kitemmodels/src/qml/plugin.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(Plugin)

#endif

//

#ifdef CHALLAH_VENDORED_QQC2_BREEZE_STYLE

#include "../vendor/qqc2-breeze-style/style/qqc2breezestyleplugin.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(QQC2BreezeStylePlugin)

#endif

#include <QtSvg>

Q_DECLARE_METATYPE(Future<ChannelsModel*>)
Q_DECLARE_METATYPE(Future<MembersModel*>)
Q_DECLARE_METATYPE(Future<MessagesModel*>)
Q_DECLARE_METATYPE(Future<InviteModel*>)
Q_DECLARE_METATYPE(Future<RolesModel*>)

void setupGST(int* argc, char*** argv)
{
#if CHALLAH_ENABLE_VOICE
	gst_init(argc, argv);
#endif
}

void setupQML(QQmlEngine* engine)
{
#if defined(CHALLAH_VENDORED_KIRIGAMI)
	KirigamiPlugin::getInstance().registerTypes(engine);
#else
	Q_UNUSED(engine)
#endif

#ifdef CHALLAH_VENDORED_KITEMMODELS
	Q_INIT_RESOURCE(kitemmodelsqml);
#endif

	QSvgRenderer rend;
	(void) rend;

	qRegisterMetaType<Croutons::FutureBase>();

	qRegisterMetaType<RolesModel*>();
	qRegisterMetaType<GuildList*>();
	qRegisterMetaType<GuildsStore*>();
	qRegisterMetaType<ChannelsModel*>();
	qRegisterMetaType<MembersModel*>();
	qRegisterMetaType<MembersStore*>();
	qRegisterMetaType<MessagesModel*>();
	qRegisterMetaType<MessagesStore*>();
	qRegisterMetaType<InviteModel*>();
	qRegisterMetaType<Utils*>();
	qRegisterMetaType<OverridesModel*>();
	qRegisterMetaType<OwnPermissionsStore*>();

#if CHALLAH_ENABLE_VOICE
	qRegisterMetaType<voiceCall*>();
#endif

	engine->rootContext()->setContextProperty("tStyleName", QQuickStyle::name());

	qRegisterMetaType<Future<ChannelsModel*>>();
	qRegisterMetaType<Future<MembersModel*>>();
	qRegisterMetaType<Future<MessagesModel*>>();
	qRegisterMetaType<Future<InviteModel*>>();
	qRegisterMetaType<Future<RolesModel*>>();

	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Challah", 1, 0, "CState", [](QQmlEngine* q, QJSEngine*) -> QObject* { return new State(q); });
	qmlRegisterSingletonType<Utils>("com.github.HarmonyDevelopment.Challah", 1, 0, "UIUtils", [](QQmlEngine* q, QJSEngine*) -> QObject* { return new Utils(q); });
	qmlRegisterType<OverlappingPanels>("com.github.HarmonyDevelopment.Challah", 1, 0, "OverlappingPanels");
	qmlRegisterType<Conditional>("com.github.HarmonyDevelopment.Challah", 1, 0, "Conditional");
	qmlRegisterType<ChallahQmlRelationalListener>("com.github.HarmonyDevelopment.Challah", 1, 0, "RelationalListener");
	qmlRegisterUncreatableType<CopyInterceptor>("com.github.HarmonyDevelopment.Challah", 1, 0, "Clipboard", "You cannot create an instance of Clipboard.");
}
