#include "roles.hpp"

#include "protos.hpp"

#include "client.hpp"
#include "util.hpp"
#include "permissions.hpp"

#include <QColor>

#define theHeaders {{"Authorization", client->userToken}}

struct RolesModel::Private
{
	QList<protocol::chat::v1::Role> roles;
};

RolesModel::RolesModel(QString homeserver, quint64 guildID) : QAbstractListModel(), homeServer(homeserver), guildID(guildID)
{
	client = Client::instanceForHomeserver(homeServer);
	d = new Private;

	protocol::chat::v1::GetGuildRolesRequest req;
	req.set_guild_id(guildID);

	auto result = client->chatKit->GetGuildRoles(req, theHeaders);
	if (!resultOk(result)) {
		return;
	}
	auto resp = unwrap(result);

	auto roles = resp.roles();
	for (auto role : roles) {
		d->roles << role;
	}
}

RolesModel::~RolesModel() {
	delete d;
}

void RolesModel::moveRoleFromTo(int from, int to)
{
	auto fromRole = d->roles[from];

	protocol::chat::v1::MoveRoleRequest req;
	req.set_guild_id(guildID);
	req.set_role_id(fromRole.role_id());

	if (to == 0) {
		req.set_before_id(d->roles[0].role_id());
	} else if (to + 1 == d->roles.length()) {
		req.set_after_id(d->roles[to].role_id());
	} else {
		req.set_after_id(d->roles[to-1].role_id());
		req.set_before_id(d->roles[to+1].role_id());
	}

	client->chatKit->MoveRole(req, theHeaders);
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
		return QVariant::fromValue(new PermissionsModel(homeServer, guildID, d->roles[index.row()].role_id()));
	}

	return QVariant();
}

QVariant RolesModel::everyonePermissions() const
{
	return QVariant::fromValue(new PermissionsModel(homeServer, guildID, 0));
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
			req.set_guild_id(guildID);
			req.set_allocated_role(new protocol::chat::v1::Role);
			req.set_modify_name(true);
			req.mutable_role()->set_name(value.toString().toStdString());

			return resultOk(client->chatKit->ModifyGuildRole(req));
		}
	}

	return false;
}

bool RolesModel::createRole(const QString& name, const QColor& colour)
{
	protocol::chat::v1::AddGuildRoleRequest req;
	req.set_guild_id(guildID);
	req.set_allocated_role(new protocol::chat::v1::Role);
	req.mutable_role()->set_name(name.toStdString());
	req.mutable_role()->set_color(colour.rgba());

	auto result = client->chatKit->AddGuildRole(req, theHeaders);
	if (!resultOk(result)) {
		return false;
	}
	auto resp = unwrap(result);

	beginInsertRows(QModelIndex(), d->roles.length(), d->roles.length());
	req.mutable_role()->set_role_id(resp.role_id());
	d->roles << req.role();
	endInsertRows();

	return true;
}

QHash<int,QByteArray> RolesModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[NameRole] = "roleName";
	ret[ColorRole] = "roleColour";
	ret[Permissions] = "permissions";

	return ret;
}
