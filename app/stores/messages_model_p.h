#pragma once

#include "messages_model.h"
#include "messages.h"

struct MessagesModel::Private
{
	QScopedPointer<MessagesStore, QScopedPointerDeleteLater> store;
	QList<quint64> messageIDs;
	bool canFetchMore = true;
	quint64 guildID;
	quint64 channelID;
	bool isFetching = false;
};
