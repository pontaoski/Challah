#pragma once

#include <QTimer>

#include "messages_model.h"
#include "messages.h"

struct MessagesModel::Private
{
	QScopedPointer<MessagesStore, QScopedPointerDeleteLater> store;
	QList<quint64> messageIDs;
	QList<quint64> typingUsers;
	QMap<quint64, QTimer*> typingTimers;
	bool canFetchMore = true;
	quint64 guildID;
	quint64 channelID;
	bool isFetching = false;
	QTimer* typingTimer;
};
