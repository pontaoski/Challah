#pragma once

#include <QObject>
#include <QJsonObject>

class QQuickTextDocument;
class QQuickItem;
class State;

class Utils : public QObject
{

	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

public:
	Utils(QObject* parent = nullptr);
	~Utils();

	Q_INVOKABLE QString formattedSize(int size);
	Q_INVOKABLE void formatDocument(State* s, QQuickTextDocument* txt, QQuickItem* field, QJsonObject obj);

};
