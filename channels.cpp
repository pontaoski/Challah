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
			} else if (msg.has_sent_message()) {
				auto sent = msg.sent_message();
				auto chanID = sent.message().location().channel_id();
				if (models.contains(chanID)) {
					QCoreApplication::postEvent(models[chanID], new MessageSentEvent(sent));
				}
			}
		}
	});
}

void ChannelsModel::customEvent(QEvent *event)
{
	if (event->type() == ChannelAddEvent::typeID) {
		auto ev = reinterpret_cast<ChannelAddEvent*>(event);
		auto idx = (std::find_if(channels.begin(), channels.end(), [=](Channel& chan) { return chan.channelID == ev->data.previous_id(); }) - channels.begin());
		idx++;
		beginInsertRows(QModelIndex(), idx, idx);
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
	case MessageModelRole:
		auto id = channels[index.row()].channelID;
		if (!models.contains(id)) {
			models[id] = new MessagesModel(const_cast<ChannelsModel*>(this), homeServer, guildID, id);
		}
		return QVariant::fromValue(models[id]);
	}

	return QVariant();
}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[ChannelIDRole] = "channelID";
	ret[ChannelNameRole] = "channelName";
	ret[ChannelIsCategoryRole] = "isCategory";
	ret[MessageModelRole] = "messagesModel";

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

	quint64 last = 0;

	for (auto channel : channels) {
		last = channel.channelID;
		if (channel.isCategory) {
			break;
		}
	}

	protocol::core::v1::CreateChannelRequest req;
	req.set_allocated_location(Location {
		.guildID = this->guildID
	});
	req.set_channel_name(name.toStdString());
	req.set_previous_id(last);

	protocol::core::v1::CreateChannelResponse resp;
	return checkStatus(client->coreKit->CreateChannel(&ctx, req, &resp));
}

QString ChannelsModel::userName(quint64 id)
{
	if (!users.contains(id)) {
		doContext;

		protocol::profile::v1::GetUserRequest req;
		req.set_user_id(id);

		protocol::profile::v1::GetUserResponse resp;

		checkStatus(client->profileKit->GetUser(&ctx, req, &resp));

		users[id] = QString::fromStdString(resp.user_name());
	}
	return users[id];
}
