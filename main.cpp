// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "avatar.hpp"
#include "state.hpp"
#include "channels.hpp"
#include "invites.hpp"
#include "overlappingpanels.hpp"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() * 8);

	auto app = new QApplication(argc, argv);

	qmlRegisterType<OverlappingPanels>("com.github.HarmonyDevelopment.Staccato", 1, 0, "OverlappingPanels");
	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Staccato", 1, 0, "HState", [](QQmlEngine*, QJSEngine*) -> QObject* { return new State; });
	qmlRegisterSingletonType<AvatarPrivate>("com.github.HarmonyDevelopment.Staccato", 1, 0, "AvatarPrivate", [] (QQmlEngine*, QJSEngine*) -> QObject* { return new AvatarPrivate; });
	qmlRegisterType<AvatarGroup>("com.github.HarmonyDevelopment.Staccato", 1, 0, "AvatarGroup");
	qmlRegisterUncreatableType<ChannelsModel>("com.github.HarmonyDevelopment.ChannelsModel", 1, 0, "ChannelsModel", "You cannot create an instance of ChannelsModel.");
	qmlRegisterUncreatableType<InviteModel>("com.github.HarmonyDevelopment.InviteModel", 1, 0, "InviteModel", "You cannot create an instance of InviteModel.");

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
