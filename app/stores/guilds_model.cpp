#include "guilds_p.h"

enum Roles {
	GuildID,
	GuildHost,
};

GuildList::GuildList(State* parent) : QAbstractListModel(parent), d(new Private), s(parent)
{
	auto toHost = [this](const std::string& it) {
		if (it == "") {
			return s->api()->mainClient()->homeserver();
		}

		return QString::fromStdString(it);
	};
	s->api()->subscribeToHomeserver();
	s->api()->mainClient()->GetGuildList(protocol::chat::v1::GetGuildListRequest{}).then([this, toHost](auto resp) {
		if (!resp.ok()) {
			return;
		}
		protocol::chat::v1::GetGuildListResponse it = resp.value();
		beginResetModel();
		for (const auto& g : it.guilds()) {
			d->guilds << qMakePair(toHost(g.server_id()), g.guild_id());
		}
		endResetModel();
	});
	connect(s->api(), &SDK::ClientManager::hsEvent, this, [this, toHost](protocol::chat::v1::StreamEvent ev) {
		if (ev.has_guild_added_to_list()) {
			auto it = ev.guild_added_to_list();

			beginInsertRows(
				QModelIndex(),
				d->guilds.count(),
				d->guilds.count()
			);
			d->guilds << qMakePair(toHost(it.homeserver()), it.guild_id());
			endInsertRows();

		} else if (ev.has_guild_removed_from_list()) {
			auto it = ev.guild_removed_from_list();
			auto id = it.guild_id();
			auto homeserver = toHost(it.homeserver());

			auto idx = -1;
			for (const auto& guild : d->guilds) {
				if (guild.second == id && guild.first == homeserver) {
					idx++;
					break;
				}
				idx++;
			}
			if (idx != -1) {
				beginRemoveRows(QModelIndex(), idx, idx);
				d->guilds.removeAt(idx);
				endRemoveRows();
			}
		}
	});
}

int GuildList::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);

	return d->guilds.count();
}

QVariant GuildList::data(const QModelIndex& idx, int role) const
{
	auto row = idx.row();
	if (row >= d->guilds.count()) {
		return QVariant();
	}

	switch (Roles(role)) {
	case Roles::GuildID:
		return QString::number(d->guilds[row].second);
	case Roles::GuildHost:
		return d->guilds[row].first;
	}

	return QVariant();
}

QHash<int, QByteArray> GuildList::roleNames() const
{
	return {
		{ Roles::GuildID, "guildID" },
		{ Roles::GuildHost, "guildHost" },
	};
}

GuildList::~GuildList()
{

}
