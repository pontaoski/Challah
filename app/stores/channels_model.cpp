#include "state.h"
#include "channels_p.h"

enum Roles {
	ID,
};

ChannelsModel::ChannelsModel(SDK::Client* c, quint64 gid, State* state) : QAbstractListModel(state), d(new Private), s(state), c(c)
{
	d->store.reset(new ChannelsStore(state, this));
	d->gid = gid;

	auto req = protocol::chat::v1::GetGuildChannelsRequest{};
	req.set_guild_id(gid);

	c->chatKit()->GetGuildChannels(req).then([this](auto r) {
		if (!r.ok()) {
			return;
		}

		protocol::chat::v1::GetGuildChannelsResponse it = r.value();
		beginResetModel();
		for (const auto& c : it.channels()) {
			d->id << c.channel_id();
			d->store->d->data[c.channel_id()] = c;
		}
		endResetModel();
	});
}

ChannelsModel::~ChannelsModel()
{

}

ChannelsStore* ChannelsModel::store()
{
	return d->store.get();
}

int ChannelsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->id.length();
}

QVariant ChannelsModel::data(const QModelIndex& index, int role) const
{
	auto idx = index.row();
	if (idx >= d->id.length()) {
		return QVariant();
	}

	switch (role) {
	case Roles::ID:
		return QString::number(d->id[idx]);
	}

	return QVariant();
}

QHash<int,QByteArray> ChannelsModel::roleNames() const
{
	return {
		{ ID, "channelID" }
	};
}

