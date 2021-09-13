#include "chat/v1/channels.pb.h"
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

	connect(s->api(), &SDK::ClientManager::chatEvent, this, [=](QString it, protocol::chat::v1::StreamEvent ev) {
		if (it != c->homeserver()) {
			return;
		}

		if (ev.has_created_channel()) {
			auto cc = ev.created_channel();
			if (cc.guild_id() != d->gid) return;

			int idx = -1;

			switch (cc.position().position_case()) {
			case protocol::harmonytypes::v1::ItemPosition::PositionCase::kTop:
				idx = 0; break;
			case protocol::harmonytypes::v1::ItemPosition::PositionCase::kBottom:
				idx = d->id.length() - 1; break;
			case protocol::harmonytypes::v1::ItemPosition::PositionCase::kBetween:
				idx = std::find(d->id.begin(), d->id.end(), cc.position().between().previous_id()) - d->id.begin();
				break;
			default:
				;
			}

			beginInsertRows(QModelIndex(), idx, idx);

			d->store->d->data[cc.channel_id()] = protocol::chat::v1::ChannelWithId { };
			auto& ref = d->store->d->data[cc.channel_id()];

			ref.set_channel_id(cc.channel_id());
			ref.mutable_channel()->set_channel_name(cc.name());
			ref.mutable_channel()->set_is_category(cc.is_category());

			d->id.insert(idx, cc.channel_id());
			endInsertRows();
		} else if (ev.has_edited_channel()) {
			auto cc = ev.edited_channel();
			if (cc.guild_id() != d->gid) return;

			if (cc.has_new_name()) {
				d->store->d->data[cc.channel_id()].mutable_channel()->set_channel_name(cc.new_name());
				d->store->keyDataChanged(QString::number(cc.channel_id()), {});
			}
		} else if (ev.has_channels_reordered()) {
			beginResetModel();
			d->id.clear();
			for (auto id : ev.channels_reordered().channel_ids()) {
				d->id << id;
			}
			endResetModel();
		} else if (ev.has_deleted_channel()) {
			auto cc = ev.deleted_channel();
			if (cc.guild_id() != d->gid) return;

			auto idx = std::find(d->id.begin(), d->id.end(), cc.channel_id()) - d->id.begin();
			beginRemoveRows(QModelIndex(), idx, idx);
			d->store->d->data.remove(cc.channel_id());
			d->id.removeAt(idx);
			endRemoveRows();
		}
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

void ChannelsModel::newChannel(const QString &name)
{
	auto it = protocol::chat::v1::CreateChannelRequest();
	it.set_guild_id(d->gid);
	it.set_channel_name(name.toStdString());
	Q_UNUSED(c->chatKit()->CreateChannel(it));
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

