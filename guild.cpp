// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QQmlEngine>

#include "state.hpp"
#include "guild.hpp"
#include "channels.hpp"

class GuildModel::Private
{
public:
	QCache<QPair<quint64, QString>, ChannelsModel> models = QCache<QPair<quint64, QString>, ChannelsModel>(10);
};

GuildModel::GuildModel() : QAbstractListModel(), d(new Private)
{
	static bool initted = false;
	if (!initted) {
		initted = true;
		qRegisterMetaType<Guild>();
	}
	connect(this, &GuildModel::addGuild, this, &GuildModel::addGuildHandler, Qt::QueuedConnection);
	connect(this, &GuildModel::removeGuild, this, &GuildModel::removeGuildHandler, Qt::QueuedConnection);
}

GuildModel::~GuildModel()
{
	delete d;
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

void GuildModel::removeGuildHandler(const QString &homeserver, quint64 id)
{
	auto idx = -1;
	for (auto& guild : guilds) {
		if (guild.guildID == id && guild.homeserver == homeserver) {
			idx++;
			break;
		}
		idx++;
	}
	if (idx != -1) {
		beginRemoveRows(QModelIndex(), idx, idx);
		guilds.removeAt(idx);
		endRemoveRows();
	}
}

int GuildModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return guilds.count();
}

void GuildModel::customEvent(QEvent *event)
{
	if (event->type() == GuildListUpdateEvent::typeID) {
		auto ev = reinterpret_cast<GuildListUpdateEvent*>(event);
		auto& data = ev->data;

		if (data.name.has_value()) {
			int i = 0;
			for (auto& guild : guilds) {
				if (guild.guildID == data.guildID && guild.homeserver == data.homeserver) {
					guild.name = data.name.value();

					dataChanged(index(i), index(i), {GuildNameRole});
					break;
				}

				i++;
			}
		}
		if (data.picture.has_value()) {
			int i = 0;
			for (auto& guild : guilds) {
				if (guild.guildID == data.guildID && guild.homeserver == data.homeserver) {
					guild.picture = data.picture.value();

					dataChanged(index(i), index(i), {GuildPictureRole});
					break;
				}

				i++;
			}
		}
	}
}

QVariant GuildModel::data(const QModelIndex &index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case GuildIDRole:
		return QString::number(guilds[index.row()].guildID);
	case GuildNameRole:
		return guilds[index.row()].name;
	case IsOwnerRole:
		return guilds[index.row()].ownerID == Client::instanceForHomeserver(guilds[index.row()].homeserver)->userID;
	case HomeserverRole:
		return guilds[index.row()].homeserver;
	case PictureRole:
		return State::instance()->transformHMCURL(guilds[index.row()].picture, guilds[index.row()].homeserver);
	case ChannelModelRole:
		auto key = qMakePair(guilds[index.row()].guildID, guilds[index.row()].homeserver);
		if (!d->models.contains(key)) {
			auto hs = guilds[index.row()].homeserver;
			if (hs.isEmpty()) {
				hs = State::instance()->client->homeserver;
			}
			d->models.insert(key, new ChannelsModel(hs, guilds[index.row()].guildID));
			QQmlEngine::setObjectOwnership(d->models[key], QQmlEngine::CppOwnership);
		}
		return QVariant::fromValue(d->models[key]);
	}

	return QVariant();
}

QHash<int, QByteArray> GuildModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[GuildIDRole] = "guildID";
	ret[GuildNameRole] = "guildName";
	ret[ChannelModelRole] = "channelModel";
	ret[IsOwnerRole] = "isOwner";
	ret[HomeserverRole] = "homeserver";
	ret[PictureRole] = "picture";

	return ret;
}
