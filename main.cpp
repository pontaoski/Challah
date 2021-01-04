// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifdef Q_OS_ANDROID

#include <QtAndroid>
#include <QGuiApplication>
#include <QQuickStyle>

// WindowManager.LayoutParams
#define FLAG_TRANSLUCENT_STATUS 0x04000000
#define FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS 0x80000000
// View
#define SYSTEM_UI_FLAG_LIGHT_STATUS_BAR 0x00002000

#else

#include <QApplication>

#endif

#include <QCoreApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "state.hpp"
#include "channels.hpp"
#include "invites.hpp"
#include "overlappingpanels.hpp"
#include "roles.hpp"
#include "permissions.hpp"
#include "promise.hpp"
#include "loginmanager.hpp"
#include "copyinterceptor.hpp"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() * 8);

#ifdef Q_OS_ANDROID
	auto app = new QGuiApplication(argc, argv);
#else
	auto app = new QApplication(argc, argv);
#endif

#ifdef Q_OS_ANDROID
	QQuickStyle::setStyle(QStringLiteral("Material"));
#elif defined(Q_OS_LINUX)
	QApplication::setStyle("Breeze");
	QIcon::setThemeName("breeze");
	QQuickStyle::setStyle("org.kde.desktop");
#endif

	qmlRegisterType<OverlappingPanels>("com.github.HarmonyDevelopment.Staccato", 1, 0, "OverlappingPanels");
	qmlRegisterType<LoginManager>("com.github.HarmonyDevelopment.Staccato", 1, 0, "LoginManager");
	qmlRegisterUncreatableType<CopyInterceptor>("com.github.HarmonyDevelopment.Staccato", 1, 0, "Clipboard", "You cannot create an instance of Clipboard.");
	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Staccato", 1, 0, "HState", [](QQmlEngine *, QJSEngine *) -> QObject * { return new State; });
	qmlRegisterUncreatableType<ChannelsModel>("com.github.HarmonyDevelopment.ChannelsModel", 1, 0, "ChannelsModel", "You cannot create an instance of ChannelsModel.");
	qmlRegisterUncreatableType<InviteModel>("com.github.HarmonyDevelopment.InviteModel", 1, 0, "InviteModel", "You cannot create an instance of InviteModel.");
	qmlRegisterUncreatableType<RolesModel>("com.github.HarmonyDevelopment.RolesModel", 1, 0, "RolesModel", "You cannot create an instance of RolesModel.");
	qmlRegisterUncreatableType<PermissionsModel>("com.github.HarmonyDevelopment.PermissionsModel", 1, 0, "PermissionsModel", "You cannot create an instance of PermissionsModel.");
	qmlRegisterUncreatableType<Promise>("com.github.HarmonyDevelopment.Promise", 1, 0, "Promise", "You cannot create an instance of Promise.");

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app->installTranslator(&qtTranslator);

	QTranslator kalamaTranslator;
	kalamaTranslator.load("Kalama_" + QLocale::system().name(), ":/po/");
	app->installTranslator(&kalamaTranslator);

	QApplication::setWindowIcon(QIcon::fromTheme(QString("io.harmonyapp.Kalama")));
	QApplication::setDesktopFileName("io.harmonyapp.Kalama.desktop");
	QApplication::setOrganizationName("Harmony Development");
	QApplication::setOrganizationDomain("io.harmonyapp");

	QQmlApplicationEngine engine;
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(
		&engine, &QQmlApplicationEngine::objectCreated,
		app, [url](QObject *obj, const QUrl &objUrl) {
			if ((obj == nullptr) && url == objUrl)
			{
				QCoreApplication::exit(-1);
			}
		},
		Qt::QueuedConnection);
	engine.load(url);

#ifdef Q_OS_ANDROID
	QtAndroid::runOnAndroidThread([=]() {
		QAndroidJniObject window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
		window.callMethod<void>("addFlags", "(I)V", FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
		window.callMethod<void>("clearFlags", "(I)V", FLAG_TRANSLUCENT_STATUS);
		window.callMethod<void>("setStatusBarColor", "(I)V", QColor("#2196f3").rgba());
		window.callMethod<void>("setNavigationBarColor", "(I)V", QColor("#2196f3").rgba());
	});
#endif

	return QApplication::exec();
}
