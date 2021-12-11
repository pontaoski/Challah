#pragma once

#include "members_store.h"

struct MembersStore::Private
{
	QMap<QString,QList<quint64>> toBatch;
	QMap<QString,QMap<quint64, protocol::profile::v1::Profile>> data;
	std::function<void()> debounced;
};
