#pragma once

#include "members_model.h"

struct MembersModel::Private
{
	quint64 gid;
	QList<quint64> id;
};
