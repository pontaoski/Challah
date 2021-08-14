#include <QLocale>
#include <QDebug>

#include "utils.h"

Utils::Utils(QObject* parent) : QObject(parent)
{
	qWarning() << "new utils" << metaObject()->className();
}

QString Utils::formattedSize(int size)
{
	return QLocale().formattedDataSize(size, 1);
}
