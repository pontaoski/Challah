#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>

#include "setup.hpp"

class Setup : public QObject
{
	Q_OBJECT

public:
	Setup() {}

public slots:
	void qmlEngineAvailable(QQmlEngine *engine)
	{
		setupQML();
	}
};

QUICK_TEST_MAIN_WITH_SETUP(tst, Setup)

#include "tst.moc"
