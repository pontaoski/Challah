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

	c->chatKit()->GetGuildRoles(
		[=](auto r) {
			if (!resultOk(r)) {
				return;
			}
			auto resp = unwrap(r);

			beginResetModel();
			auto roles = resp.roles();
			for (auto role : roles) {
				d->roles << role;
			}
			endResetModel();
		},
		req
	);
}

RolesModel::~RolesModel() {
}

void RolesModel::moveRoleFromTo(int from, int to)
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

	c->chatKit()->MoveRole(
		[=](auto r) {
			Q_UNUSED(r)
		}, req);
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

			c->chatKit()->ModifyGuildRole([](auto) {}, req);
			return true;
		}
	}

	return false;
}

QIviPendingReply<bool> RolesModel::createRole(const QString& name, const QColor& colour)
{
	auto reply = QIviPendingReply<bool>();

	protocol::chat::v1::AddGuildRoleRequest req;
	req.set_guild_id(d->guildID);
	req.set_allocated_role(new protocol::chat::v1::Role);
	req.mutable_role()->set_name(name.toStdString());
	req.mutable_role()->set_color(colour.rgba());

	c->chatKit()->AddGuildRole(
		[this, k = reply, role = req.role()](auto r) {
			auto reply = k;
			if (!resultOk(r)) {
				reply.setSuccess(false);
				return;
			}
			auto resp = unwrap(r);

			beginInsertRows(QModelIndex(), d->roles.length(), d->roles.length());
			auto cp = role;
			cp.set_role_id(resp.role_id());
			d->roles << cp;
			endInsertRows();

			reply.setSuccess(true);
		},
		req
	);

	return reply;
}

QHash<int,QByteArray> RolesModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[NameRole] = "roleName";
	ret[ColorRole] = "roleColour";
	ret[Permissions] = "permissions";

	return ret;
}
