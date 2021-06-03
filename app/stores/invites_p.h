#pragma once

#include "invites.h"

struct Invite {
	QString id;
	qint32 possibleUses;
	qint32 useCount;
};

struct InviteModel::Private
{
	quint64 guildID;
	QList<Invite> invites;
};
