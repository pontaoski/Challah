#pragma once

#include "channels.h"

struct ChannelsModel::Private {
	QScopedPointer<ChannelsStore, QScopedPointerDeleteLater> store;
	quint64 gid;
	QList<quint64> id;
};

struct ChannelsStore::Private {
	QMap<quint64,protocol::chat::v1::ChannelWithId> data;
};
