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
	const auto [hs, id] = fromVariant(key);

	s->api()->clientForHomeserver(hs).then([key, this](auto c) {
		const auto [hs, id] = fromVariant(key);

		auto req = protocol::chat::v1::GetGuildRequest{};
		req.set_guild_id(id);

		c->chatKit()->GetGuild(req).then([this, id = qMakePair(hs, id)](auto resp) {
			if (!resultOk(resp)) {
				return;
			}
			auto it = unwrap(resp);
			d->guilds[id] = it;
			Q_EMIT keyAdded(toVariant(id));
		});
	});
}

QVariant GuildsStore::data(const QVariant& key, int role)
{
	if (!checkKey(key)) {
		return QVariant();
	}

	switch (Roles(role)) {
	case Roles::Name:
		return QString::fromStdString(d->guilds[fromVariant(key)].guild().name());
	case Roles::Picture:
		return s->mediaURL(QString::fromStdString(d->guilds[fromVariant(key)].guild().picture()), fromVariant(key).first);
	}

	return QVariant();
}
