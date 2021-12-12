#include <QColor>

#include "roles.h"
#include "client.h"
#include "permissions.h"

enum Roles {
	NameRole = Qt::UserRole,
	ColorRole,
	Permissions,
	IDRole,
};

struct RolesModel::Private
{
	quint64 guildID;
	QList<protocol::chat::v1::RoleWithId> roles;
	bool working = false;
};

RolesModel::RolesModel(QString host, quint64 guildID, State* state) : QAbstractListModel(state), d(new Private), s(state), host(host)
{
	d->guildID = guildID;

	protocol::chat::v1::GetGuildRolesRequest req;
	req.set_guild_id(guildID);

	s->api()->dispatch(host, &SDK::R::GetGuildRoles, req).then([this](auto r) {
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

	req.set_allocated_new_position(new protocol::harmonytypes::v1::ItemPosition);
	auto pos = req.mutable_new_position();
	if (to == 0) {
		pos->set_position(protocol::harmonytypes::v1::ItemPosition::POSITION_BEFORE_UNSPECIFIED);
		pos->set_item_id(d->roles.first().role_id());
	} else if (to + 1 == d->roles.length()) {
		pos->set_position(protocol::harmonytypes::v1::ItemPosition::POSITION_AFTER);
		pos->set_item_id(d->roles.last().role_id());
	} else {
		pos->set_position(protocol::harmonytypes::v1::ItemPosition::POSITION_BEFORE_UNSPECIFIED);
		pos->set_item_id(d->roles[to].role_id());
	}

	auto r = co_await s->api()->dispatch(host, &SDK::R::MoveRole, req);
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
	case IDRole:
		return QString::number(d->roles[index.row()].role_id());
	case NameRole:
		return QString::fromStdString(d->roles[index.row()].role().name());
	case ColorRole:
		return QColor::fromRgba(d->roles[index.row()].role().color());
	case Permissions:
		return QVariant::fromValue(new PermissionsModel(host, d->guildID, d->roles[index.row()].role_id(), s));
	}

	return QVariant();
}

QVariant RolesModel::everyonePermissions() const
{
	return QVariant::fromValue(new PermissionsModel(host, d->guildID, 0, s));
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
			req.set_role_id(d->roles[index.row()].role_id());
			req.set_new_name(value.toString().toStdString());

			(void) s->api()->dispatch("", &SDK::R::ModifyGuildRole, req);

			return true;
		}
	}

	return false;
}

FutureBase RolesModel::createRole(QString name, QColor colour)
{
	protocol::chat::v1::AddGuildRoleRequest req;
	req.set_guild_id(d->guildID);
	req.set_name(name.toStdString());
	req.set_color(colour.rgba());

	auto response = co_await s->api()->dispatch(host, &SDK::R::AddGuildRole, req);
	if (!response.ok()) {
		co_return false;
	}
	auto resp = response.value();

	beginInsertRows(QModelIndex(), d->roles.length(), d->roles.length());
	auto role = protocol::chat::v1::RoleWithId{};
	role.set_role_id(resp.role_id());
	role.set_allocated_role(new protocol::chat::v1::Role);
	role.mutable_role()->set_name(name.toStdString());
	role.mutable_role()->set_color(colour.rgba());
	d->roles << role;
	endInsertRows();

	co_return true;
}

Croutons::FutureBase RolesModel::moveRole(const QString& sid, int to)
{
	const auto id = sid.toULongLong();
	int idx = 0;
	bool ok = false;
	for (const auto& role : d->roles) {
		if (role.role_id() == id) {
			ok = true;
			break;
		}
		idx++;
	}
	if (ok) {
		d->working = true;
		Q_EMIT workingChanged();
		co_await moveRoleFromTo(idx, to);
		d->working = false;
		Q_EMIT workingChanged();
	}
	co_return {};
}

bool RolesModel::working() const
{
	return d->working;
}

QHash<int,QByteArray> RolesModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[IDRole] = "roleID";
	ret[NameRole] = "roleName";
	ret[ColorRole] = "roleColour";
	ret[Permissions] = "permissions";

	return ret;
}
