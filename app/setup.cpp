#include "channels.hpp"
#include "copyinterceptor.hpp"
#include "invites.hpp"
#include "loginmanager.hpp"
#include "overlappingpanels.hpp"
#include "permissions.hpp"
#include "promise.hpp"
#include "roles.hpp"
#include "state.hpp"
#include "userroles.hpp"
#include "conditional.hpp"

#include "setup.hpp"

#ifdef CHALLAH_VENDORED_KIRIGAMI

#define KIRIGAMI_BUILD_TYPE_STATIC

#include "../vendor/kirigami/src/kirigamiplugin.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(KirigamiPlugin)

#undef KIRIGAMI_BUILD_TYPE_STATIC

#endif

#ifdef CHALLAH_VENDORED_QQC2_BREEZE_STYLE

#include "../vendor/qqc2-breeze-style/style/qqc2breezestyleplugin.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(QQC2BreezeStylePlugin)

#endif

void setupQML(QQmlEngine* engine)
{
#if defined(CHALLAH_VENDORED_KIRIGAMI)
	KirigamiPlugin::getInstance().registerTypes(engine);
#else
	Q_UNUSED(engine)
#endif

	qmlRegisterType<OverlappingPanels>("com.github.HarmonyDevelopment.Staccato", 1, 0, "OverlappingPanels");
	qmlRegisterType<LoginManager>("com.github.HarmonyDevelopment.Staccato", 1, 0, "LoginManager");
	qmlRegisterType<Conditional>("com.github.HarmonyDevelopment.Staccato", 1, 0, "Conditional");
	qRegisterMetaType<MessagesModel*>();
	qRegisterMetaType<UserRolesModel*>();
	qmlRegisterType(QUrl("qrc:/Main.qml"), "com.github.HarmonyDevelopment.Staccato.Tests", 1, 0, "MainWindow");
	qmlRegisterUncreatableType<CopyInterceptor>("com.github.HarmonyDevelopment.Staccato", 1, 0, "Clipboard", "You cannot create an instance of Clipboard.");
	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Staccato", 1, 0, "HState", [](QQmlEngine *, QJSEngine *) -> QObject * { return new State; });
	qmlRegisterUncreatableType<ChannelsModel>("com.github.HarmonyDevelopment.ChannelsModel", 1, 0, "ChannelsModel", "You cannot create an instance of ChannelsModel.");
	qmlRegisterUncreatableType<InviteModel>("com.github.HarmonyDevelopment.InviteModel", 1, 0, "InviteModel", "You cannot create an instance of InviteModel.");
	qmlRegisterUncreatableType<RolesModel>("com.github.HarmonyDevelopment.RolesModel", 1, 0, "RolesModel", "You cannot create an instance of RolesModel.");
	qmlRegisterUncreatableType<PermissionsModel>("com.github.HarmonyDevelopment.PermissionsModel", 1, 0, "PermissionsModel", "You cannot create an instance of PermissionsModel.");
	qmlRegisterUncreatableType<Promise>("com.github.HarmonyDevelopment.Promise", 1, 0, "Promise", "You cannot create an instance of Promise.");
}
