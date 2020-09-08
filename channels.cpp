#include <QtConcurrent>
#include "channels.hpp"

#include "client.hpp"
#include "util.hpp"

using grpc::ClientContext;

ChannelsModel::ChannelsModel(QString homeServer, quint64 guildID) : QAbstractListModel(), homeServer(homeServer), guildID(guildID)
{
	client = Client::instanceForHomeserver(homeServer);

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::GetGuildChannelsRequest req;
	req.set_allocated_location(Location{
		.guildID = guildID,
	});

	protocol::core::v1::GetGuildChannelsResponse resp;
	checkStatus(client->coreKit->GetGuildChannels(&ctx, req, &resp));
	resp.channels_size();

	for (auto chan : resp.channels()) {
		channels << Channel {
			.channelID = chan.channel_id(),
			.name = QString::fromStdString(chan.channel_name()),
			.isCategory = chan.is_category(),
		};
	}

	QtConcurrent::run([=]() {
		ClientContext ctx;
		client->authenticate(ctx);

		protocol::core::v1::StreamGuildEventsRequest req;
		req.set_allocated_location(Location { .guildID = guildID });
		auto stream = client->coreKit->StreamGuildEvents(&ctx, req);
		protocol::core::v1::GuildEvent msg;

		while (stream->Read(&msg)) {
			if (msg.has_created_channel()) {

			}
		}
	});
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return channels.count();
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case ChannelIDRole:
		return channels[index.row()].channelID;
	case ChannelNameRole:
		return channels[index.row()].name;
	case ChannelIsCategoryRole:
		return channels[index.row()].isCategory;
	}

	return QVariant();
}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[ChannelIDRole] = "channelID";
	ret[ChannelNameRole] = "channelName";
	ret[ChannelIsCategoryRole] = "isCategory";

	return ret;
}