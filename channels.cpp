#include <QtConcurrent>
#include "channels.hpp"

#include "client.hpp"
#include "util.hpp"

#define doContext ClientContext ctx; client->authenticate(ctx)

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
				QCoreApplication::postEvent(this, new ChannelAddEvent(msg.created_channel()));
			} else if (msg.has_deleted_channel()) {
				QCoreApplication::postEvent(this, new ChannelDeleteEvent(msg.deleted_channel()));
			}
		}
	});
}

void ChannelsModel::customEvent(QEvent *event)
{
	if (event->type() == ChannelAddEvent::typeID) {
		auto ev = reinterpret_cast<ChannelAddEvent*>(event);
		auto idx = std::find_if(channels.begin(), channels.end(), [=](Channel& chan) { return chan.channelID == ev->data.next_id(); });
		beginInsertRows(QModelIndex(), idx - channels.begin(), idx - channels.begin());
		channels.insert(idx, Channel{
			.channelID = ev->data.location().channel_id(),
			.name = QString::fromStdString(ev->data.name()),
			.isCategory = ev->data.is_category()
		});
		endInsertRows();
	} else if (event->type() == ChannelDeleteEvent::typeID) {
		auto ev = reinterpret_cast<ChannelDeleteEvent*>(event);
		auto idx = std::find_if(channels.begin(), channels.end(), [=](Channel &chan) { return chan.channelID == ev->data.location().channel_id(); });
		beginRemoveRows(QModelIndex(), idx - channels.begin(), idx - channels.begin());
		channels.removeAt(idx - channels.begin());
		endRemoveRows();
	}
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

void ChannelsModel::deleteChannel(quint64 channel)
{
	doContext;

	protocol::core::v1::DeleteChannelRequest req;
	req.set_allocated_location(Location {
		.guildID = this->guildID,
		.channelID = channel
	});
	google::protobuf::Empty resp;
	checkStatus(client->coreKit->DeleteChannel(&ctx, req, &resp));
}

bool ChannelsModel::createChannel(const QString& name)
{
	doContext;

	protocol::core::v1::CreateChannelRequest req;
	req.set_allocated_location(Location {
		.guildID = this->guildID
	});
	req.set_channel_name(name.toStdString());

	protocol::core::v1::CreateChannelResponse resp;
	return checkStatus(client->coreKit->CreateChannel(&ctx, req, &resp));
}