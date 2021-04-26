#pragma once

#include "chat/v1/guilds.pb.h"
#include "guilds.h"

struct GuildsStore::Private
{
	QMap<QPair<QString,quint64>, protocol::chat::v1::GetGuildResponse> guilds;
};

struct GuildList::Private
{
	QList<QPair<QString,quint64>> guilds;
};
