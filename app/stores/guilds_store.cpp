#include <QJsonArray>

#include "client.h"

#include "guilds.h"
#include "guilds_p.h"
#include "uploading.h"

enum Roles
{
	Name,
	Picture,
};

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

GuildsStore::GuildsStore(State* parent) : ChallahAbstractRelationalModel(parent), d(new Private), s(parent)
{
	connect(s->api(), &SDK::ClientManager::chatEvent, this, [=](QString host, protocol::chat::v1::StreamEvent event) {
		if (!event.has_edited_guild()) {
			return;
		}
		const auto e = event.edited_guild();
		auto g = qMakePair(host, e.guild_id());
		if (!d->guilds.contains(g)) {
			return;
		}

		if (e.has_new_name()) {
			d->guilds[g].mutable_guild()->set_name(e.new_name());

		}
		if (e.has_new_picture()) {
			d->guilds[g].mutable_guild()->set_picture(e.new_picture());
		}

		Q_EMIT keyDataChanged(toVariant(g), {});
	});
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

bool GuildsStore::checkKey(const QVariant& key)
{
	auto it = fromVariant(key);
	return d->guilds.contains(it);
}

bool GuildsStore::canFetchKey(const QVariant& key)
{
	const auto [hs, id] = fromVariant(key);

	if (hs.isEmpty() || id == 0) {
		return false;
	}

	return true;
}

void GuildsStore::fetchKey(const QVariant& key)
{
	const auto [hs, id] = fromVariant(key);

	auto req = protocol::chat::v1::GetGuildRequest{};
	req.set_guild_id(id);

	s->api()->dispatch(hs, &SDK::R::GetGuild, req).then([this, id = qMakePair(hs, id)](auto resp) {
		if (!resultOk(resp)) {
			return;
		}
		auto it = unwrap(resp);
		d->guilds[id] = it;
		Q_EMIT keyAdded(toVariant(id));
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
		if (d->guilds[fromVariant(key)].guild().picture().empty()) {
			return QString();
		}
		return s->mediaURL(QString::fromStdString(d->guilds[fromVariant(key)].guild().picture()), fromVariant(key).first);
	}

	return QVariant();
}

void GuildsStore::setName(const QString& host, const QString& guildID, const QString& name)
{
	auto req = protocol::chat::v1::UpdateGuildInformationRequest { };
	req.set_guild_id(guildID.toULongLong());
	req.set_new_name(name.toStdString());
	s->api()->dispatch(host, &SDK::R::UpdateGuildInformation, req);
}

void GuildsStore::setPicture(const QString& host, const QString& guildID, const QUrl& photo)
{
	uploadFile(s, host, photo).then([this, host = host, id = guildID.toULongLong()](Result<QString,Error> hid) mutable {
		if (!hid.ok()) {
			return;
		}

		auto hmc = hid.value();

		auto req = protocol::chat::v1::UpdateGuildInformationRequest { };
		req.set_guild_id(id);
		req.set_new_name(hmc.toStdString());

		s->api()->dispatch(host, &SDK::R::UpdateGuildInformation, req);
	});
}
