#include <QJsonArray>

#include "client.h"

#include "guilds.h"
#include "guilds_p.h"

enum Roles
{
	Name,
	Picture,
};

GuildsStore::GuildsStore(State* parent) : ChallahAbstractRelationalModel(parent), d(new Private), s(parent)
{

}

GuildsStore::~GuildsStore()
{

}

QHash<int, QByteArray> GuildsStore::roleNames()
{
	return {
		{ Roles::Name, "name" },
		{ Roles::Picture, "picture" },
	};
}

QPair<QString,quint64> fromVariant(const QVariant& hi)
{
	const auto js = hi.value<QVariantList>();
	return qMakePair(js[0].toString(), js[1].toString().toULongLong());
}

QVariant toVariant(const QPair<QString,quint64>& it)
{
	QJsonArray arr;
	arr << it.first << QString::number(it.second);
	return arr;
}

bool GuildsStore::checkKey(const QVariant& key)
{
	auto it = fromVariant(key);
	return d->guilds.contains(it);
}

bool GuildsStore::canFetchKey(const QVariant& key)
{
	Q_UNUSED(key)

	return true;
}

void GuildsStore::fetchKey(const QVariant& key)
{
	auto [hs, id] = fromVariant(key);
	auto req = protocol::chat::v1::GetGuildRequest{};
	req.set_guild_id(id);
	s->api()->clientForHomeserver(hs)->chatKit()->GetGuild([this, id = qMakePair(hs, id)](ChatServiceServiceClient::Result<protocol::chat::v1::GetGuildResponse> resp) {
		if (!resultOk(resp)) {
			return;
		}
		auto it = unwrap(resp);
		d->guilds[id] = it;
		Q_EMIT keyAdded(toVariant(id));
	}, req);
}

QVariant GuildsStore::data(const QVariant& key, int role)
{
	if (!checkKey(key)) {
		return QVariant();
	}

	switch (Roles(role)) {
	case Roles::Name:
		return QString::fromStdString(d->guilds[fromVariant(key)].guild_name());
	case Roles::Picture:
		return QString::fromStdString(d->guilds[fromVariant(key)].guild_picture());
	}

	return QVariant();
}
