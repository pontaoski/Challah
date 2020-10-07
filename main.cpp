// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "state.hpp"
#include "channels.hpp"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount() * 8);

	auto app = new QApplication(argc, argv);

	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Staccato", 1, 0, "HState", [](QQmlEngine*, QJSEngine*) -> QObject* { return new State; });
	qmlRegisterUncreatableType<ChannelsModel>("com.github.HarmonyDevelopment.ChannelsModel", 1, 0, "ChannelsModel", "You cannot create an instance of ChannelsModel.");

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