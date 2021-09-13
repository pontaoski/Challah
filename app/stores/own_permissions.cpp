#include "own_permissions_p.h"

#include "client.h"

enum Roles
{
	Has
};

OwnPermissionsStore::OwnPermissionsStore(State* state) : ChallahAbstractRelationalModel(state), d(new Private), state(state)
{
	connect(state->api(), &SDK::ClientManager::chatEvent, this, [=](QString hs, protocol::chat::v1::StreamEvent ev) {
		if (!ev.has_permission_updated()) {
			return;
		}

		auto it = ev.permission_updated();

		auto node = Node{hs, it.guild_id(), it.channel_id(), QString::fromStdString(it.query())};
		d->data[node] = it.ok();

		Q_EMIT keyDataChanged(node.into(), {});
	});
}

OwnPermissionsStore::~OwnPermissionsStore()
{

}

static const QList<QString> nodes = {
	"actions.trigger",
	"channels.manage.change-information",
	"channels.manage.create",
	"channels.manage.delete",
	"channels.manage.move",
	"guild.manage.change-information",
	"guild.manage.delete",
	"invites.manage.create",
	"invites.manage.delete",
	"invites.view",
	"messages.pins.add",
	"messages.pins.remove",
	"messages.send",
	"messages.view",
	"permissions.manage.get",
	"permissions.manage.set",
	"permissions.query",
	"permissions.query",
	"roles.get",
	"roles.manage",
	"roles.user.get",
	"roles.user.manage",
	"user.manage.ban",
	"user.manage.kick",
	"user.manage.unban",
};

QVariant OwnPermissionsStore::data(const QVariant& key, int role)
{
	Q_UNUSED(role)

	auto node = Node::from(key);

	if (!d->data.contains(node)) {
		return QVariant();
	}

	return d->data[node];
}

bool OwnPermissionsStore::checkKey(const QVariant& key)
{
	auto node = Node::from(key);

	return d->data.contains(node);
}

bool OwnPermissionsStore::canFetchKey(const QVariant& key)
{
	Q_UNUSED(key)

	return true;
}

void OwnPermissionsStore::fetchKey(const QVariant& key)
{
	using namespace protocol::chat::v1;

	auto node = Node::from(key);

	if (d->fetching.contains(node)) {
		return;
	}

	d->fetching << node;

	state->api()->clientForHomeserver(node.homeserver).then([=](Result<SDK::Client*, Error> res) {
		if (!res.ok()) {
			return;
		}

		auto c = res.value();

		d->fetching.removeAll(node);

		auto req = QueryHasPermissionRequest{};
		req.set_guild_id(node.guild);
		req.set_channel_id(node.channel);
		req.set_check_for(node.node.toStdString());

		c->chatKit();

		c->chatKit()->QueryHasPermission(req).then([=](Result<QueryHasPermissionResponse, QString> r) {
			if (!r.ok()) {
				return;
			}

			auto value = r.value();
			d->data[node] = value.ok();
			Q_EMIT keyAdded(key);
		});
	});
}

QHash<int, QByteArray> OwnPermissionsStore::roleNames()
{
	return {
		{ Roles::Has, "has" }
	};
}
