// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "setup.hpp"

#include <QCoreApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QThreadPool>
#include <QLibraryInfo>
#include <QTranslator>

#ifdef Q_OS_ANDROID

#include <QtAndroid>
#include <QAndroidJniObject>
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

#if defined(Q_OS_ANDROID)
	QQuickStyle::setStyle(QStringLiteral("Material"));
#elif defined(Q_OS_LINUX) && !defined(CHALLAH_VENDORED_KIRIGAMI)
	QApplication::setStyle("Breeze");
	QIcon::setThemeName("breeze");
	QQuickStyle::setStyle("org.kde.desktop");
#endif

#ifdef CHALLAH_VENDORED_QQC2_BREEZE_STYLE
	qDebug() << "vendored";
#else
	qDebug() << "not vendored";
#endif

#ifdef CHALLAH_VENDORED_QQC2_BREEZE_STYLE
	// this works awfully
	// QQuickStyle::setStyle("qrc:/org/kde/breeze");
	QQuickStyle::setStyle("Material");
#endif

	QQmlApplicationEngine engine;
	setupQML(&engine);

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app->installTranslator(&qtTranslator);

	QTranslator ChallahTranslator;
	ChallahTranslator.load("Challah_" + QLocale::system().name(), ":/po/");
	app->installTranslator(&ChallahTranslator);

#ifndef Q_OS_ANDROID
	QApplication::setWindowIcon(QIcon::fromTheme(QString("io.harmonyapp.Challah")));
#endif
	QGuiApplication::setDesktopFileName("io.harmonyapp.Challah.desktop");
	QCoreApplication::setOrganizationName("Harmony Development");
	QCoreApplication::setOrganizationDomain("io.harmonyapp");

	const QUrl url(QStringLiteral("qrc:/Main.qml"));
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

	return QCoreApplication::exec();
}
