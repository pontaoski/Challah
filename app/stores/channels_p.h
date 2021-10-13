#pragma once

#include "channels.h"

struct ChannelsModel::Private {
	QScopedPointer<ChannelsStore, QScopedPointerDeleteLater> store;
	quint64 gid;
	QString host;
	bool working = false;
	QList<quint64> id;
};

struct ChannelsStore::Private {
	ChannelsModel* cm;
	QMap<quint64,protocol::chat::v1::ChannelWithId> data;
};
