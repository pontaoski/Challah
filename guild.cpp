#include "guild.hpp"
#include "channels.hpp"

GuildModel::GuildModel() : QAbstractListModel()
{
	static bool initted = false;
	if (!initted) {
		initted = true;
		qRegisterMetaType<Guild>();
	}
	connect(this, &GuildModel::addGuild, this, &GuildModel::addGuildHandler, Qt::QueuedConnection);
}

void GuildModel::addGuildHandler(Guild guild)
{
	beginInsertRows(
		QModelIndex(),
		guilds.count(),
		guilds.count()
	);
	guilds << guild;
	endInsertRows();
}

int GuildModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return guilds.count();
}

QVariant GuildModel::data(const QModelIndex &index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case GuildIDRole:
		return guilds[index.row()].guildID;
	case GuildNameRole:
		return guilds[index.row()].name;
	case ChannelModelRole:
		return QVariant::fromValue(new ChannelsModel(guilds[index.row()].homeserver, guilds[index.row()].guildID));
	}

	return QVariant();
}

QHash<int, QByteArray> GuildModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[GuildIDRole] = "guildID";
	ret[GuildNameRole] = "guildName";
	ret[ChannelModelRole] = "channelModel";

	return ret;
}