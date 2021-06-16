#include <QColor>

#include "roles.h"
#include "client.h"
#include "permissions.h"

enum Roles {
	NameRole = Qt::UserRole,
	ColorRole,
	Permissions,
};

struct RolesModel::Private
{
	quint64 guildID;
	QList<protocol::chat::v1::Role> roles;
};

RolesModel::RolesModel(SDK::Client* client, quint64 guildID, State* state) : QAbstractListModel(client), d(new Private), s(state), c(client)
{
	d->guildID = guildID;

	protocol::chat::v1::GetGuildRolesRequest req;
	req.set_guild_id(guildID);

	c->chatKit()->GetGuildRoles(req).then([this](auto r) {
		if (!r.ok()) {
			return;
		}

		auto resp = r.value();
		beginResetModel();
		auto roles = resp.roles();
		for (auto role : roles) {
			d->roles << role;
		}
		endResetModel();
	});
}

RolesModel::~RolesModel() {
}

FutureBase RolesModel::moveRoleFromTo(int from, int to)
{
	auto fromRole = d->roles[from];

	protocol::chat::v1::MoveRoleRequest req;
	req.set_guild_id(d->guildID);
	req.set_role_id(fromRole.role_id());

	if (to == 0) {
		req.set_before_id(d->roles[0].role_id());
	} else if (to + 1 == d->roles.length()) {
		req.set_after_id(d->roles[to].role_id());
	} else {
		req.set_after_id(d->roles[to-1].role_id());
		req.set_before_id(d->roles[to+1].role_id());
	}

	auto r = co_await c->chatKit()->MoveRole(req);
	co_return r.ok();
}

int RolesModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->roles.length();
}

QVariant RolesModel::data(const QModelIndex& index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case NameRole:
		return QString::fromStdString(d->roles[index.row()].name());
	case ColorRole:
		return QColor::fromRgba(d->roles[index.row()].color());
	case Permissions:
		return QVariant::fromValue(new PermissionsModel(c, d->guildID, d->roles[index.row()].role_id(), s));
	}

	return QVariant();
}

QVariant RolesModel::everyonePermissions() const
{
	return QVariant::fromValue(new PermissionsModel(c, d->guildID, 0, s));
}

bool RolesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!checkIndex(index))
		return false;

	switch (role)
	{
	case NameRole:
		{
			protocol::chat::v1::ModifyGuildRoleRequest req;
			req.set_guild_id(d->guildID);
			req.set_allocated_role(new protocol::chat::v1::Role);
			req.set_modify_name(true);
			req.mutable_role()->set_name(value.toString().toStdString());

			(void) c->chatKit()->ModifyGuildRole(req);

			return true;
		}
	}

	return false;
}

FutureBase RolesModel::createRole(const QString& name, const QColor& colour)
{
	protocol::chat::v1::AddGuildRoleRequest req;
	req.set_guild_id(d->guildID);
	req.set_allocated_role(new protocol::chat::v1::Role);
	req.mutable_role()->set_name(name.toStdString());
	req.mutable_role()->set_color(colour.rgba());

	auto response = co_await c->chatKit()->AddGuildRole(req);
	if (!response.ok()) {
		co_return false;
	}
	auto resp = response.value();

	beginInsertRows(QModelIndex(), d->roles.length(), d->roles.length());
	auto cp = req.role();
	cp.set_role_id(resp.role_id());
	d->roles << cp;
	endInsertRows();

	co_return true;
}

QHash<int,QByteArray> RolesModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[NameRole] = "roleName";
	ret[ColorRole] = "roleColour";
	ret[Permissions] = "permissions";

	return ret;
}
