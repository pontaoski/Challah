#pragma once

#include <QObject>
#include <QQmlParserStatus>

class QQuickItem;

class LoginManager : public QObject, public QQmlParserStatus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	struct Private;
	Private* d;

protected:
	void customEvent(QEvent* event) override;
	void runWork();

public:
	void classBegin() override { }
	void componentComplete() override;

	LoginManager();
	~LoginManager();
	Q_INVOKABLE void beginLogin(const QString& hs);
	Q_INVOKABLE void reparent(QQuickItem* child, QQuickItem* to);
	Q_SIGNAL void placeItem(QQuickItem* item);
};
