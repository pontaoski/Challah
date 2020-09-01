#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "state.hpp"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	auto app = new QApplication(argc, argv);

	qmlRegisterSingletonType<State>("com.github.HarmonyDevelopment.Staccato", 1, 0, "HState", [](QQmlEngine*, QJSEngine*) -> QObject* { return new State; });

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
