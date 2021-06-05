#pragma once

#include "yoinked from qt ivi/qivipendingreply.h"

class NameResolver
{

	QIviPendingReply<QString> resolveURL(const QString& it);

};
