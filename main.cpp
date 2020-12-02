// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "avatar.hpp"
#include "state.hpp"
#include "channels.hpp"
#include "invites.hpp"
#include "overlappingpanels.hpp"
#include "roles.hpp"
#include "permissions.hpp"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() * 8);

	auto app = new QApplication(argc, argv);

	qmlRegisterType<OverlappingPanels>("com.github.HarmonyDevelopment.Staccato", 1, 0, "OverlappingPanels");
	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Staccato", 1, 0, "HState", [](QQmlEngine *, QJSEngine *) -> QObject * { return new State; });
	qmlRegisterSingletonType<AvatarPrivate>("com.github.HarmonyDevelopment.Staccato", 1, 0, "AvatarPrivate", [](QQmlEngine *, QJSEngine *) -> QObject * { return new AvatarPrivate; });
	qmlRegisterType<AvatarGroup>("com.github.HarmonyDevelopment.Staccato", 1, 0, "AvatarGroup");
	qmlRegisterUncreatableType<ChannelsModel>("com.github.HarmonyDevelopment.ChannelsModel", 1, 0, "ChannelsModel", "You cannot create an instance of ChannelsModel.");
	qmlRegisterUncreatableType<InviteModel>("com.github.HarmonyDevelopment.InviteModel", 1, 0, "InviteModel", "You cannot create an instance of InviteModel.");
	qmlRegisterUncreatableType<RolesModel>("com.github.HarmonyDevelopment.RolesModel", 1, 0, "RolesModel", "You cannot create an instance of RolesModel.");
	qmlRegisterUncreatableType<PermissionsModel>("com.github.HarmonyDevelopment.PermissionsModel", 1, 0, "PermissionsModel", "You cannot create an instance of PermissionsModel.");

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app->installTranslator(&qtTranslator);

	QTranslator murmurTranslator;
	murmurTranslator.load("Murmur_" + QLocale::system().name(), ":/po/");
	app->installTranslator(&murmurTranslator);

	QApplication::setWindowIcon(QIcon::fromTheme(QString("com.github.harmony-development.Murmur")));
	QApplication::setDesktopFileName("com.github.harmony-development.Murmur.desktop");
	QApplication::setStyle("Breeze");
	QIcon::setThemeName("breeze");
	QQuickStyle::setStyle("org.kde.desktop");

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

	return QApplication::exec();
}
