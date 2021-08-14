#pragma once

#include <QObject>

class Utils : public QObject
{

	Q_OBJECT

public:
	Utils(QObject* parent = nullptr);

	Q_INVOKABLE QString formattedSize(int size);

};
