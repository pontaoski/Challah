#pragma once

#include <QColor>

#include "qjsonvalue.h"
#include "userroles.hpp"

class Client;

struct RoleData
{
	QString name;
	QColor color;
	bool hoist;
	bool pingable;
	quint64 id;
};

struct UserRolesModel::Private
{
	Client* client = nullptr;

	bool errored = false;
	bool editable = false;

	quint64 userID = 0;
	quint64 guildID = 0;

	QMap<quint64,RoleData> guildRoles;
	QList<quint64> userRoles;
};
