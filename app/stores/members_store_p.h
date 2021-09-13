#pragma once

#include "members_store.h"

struct MembersStore::Private
{
	QMap<QString,QMap<quint64, protocol::profile::v1::Profile>> data;
};
