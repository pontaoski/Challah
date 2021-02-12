#include "chat/v1/permissions.pb.h"

#include "client.hpp"

#include "qjsonarray.h"
#include "qjsonobject.h"
#include "qjsonvalue.h"
#include "userroles.hpp"
#include "userroles_p.hpp"
#include "util.hpp"

#include <algorithm>

#include <QtConcurrent>

#define theHeaders {{"Authorization", d->client->userToken}}

UserRolesModel::UserRolesModel(
	quint64 userID,
	quint64 guildID,
	const QString& homeserver,
	QObject* parent
)
: QAbstractListModel(parent), d(new Private)
{
	d->client = Client::instanceForHomeserver(homeserver);
	d->userID = userID;
	d->guildID = guildID;

	protocol::chat::v1::GetUserRolesRequest req;
	req.set_user_id(userID);
	req.set_guild_id(guildID);

	auto result = d->client->chatKit->GetUserRoles(req, theHeaders);
	if (!resultOk(result)) {
		d->errored = true;
		return;
	}
	auto resp = unwrap(result);

	protocol::chat::v1::GetGuildRolesRequest req2;
	req2.set_guild_id(guildID);

	auto result2 = d->client->chatKit->GetGuildRoles(req2, theHeaders);
	if (!resultOk(result2)) {
		d->errored = true;
		return;
	}
	auto resp2 = unwrap(result2);

	protocol::chat::v1::QueryPermissionsRequest req3;
	req3.set_guild_id(guildID);

	auto result3 = d->client->chatKit->QueryHasPermission(req3, theHeaders);
	if (!resultOk(result3)) {
		d->editable = false;
	} else {
		d->editable = unwrap(result3).ok();
	}

	for (auto guildrole : resp2.roles()) {
		d->guildRoles[guildrole.role_id()] = RoleData {
			QString::fromStdString(guildrole.name()),
			QColor::fromRgba(guildrole.color()),
			guildrole.hoist(),
			guildrole.pingable(),
			guildrole.role_id(),
		};
	}

	for (auto role : resp.roles()) {
		d->userRoles << role;
	}
}

bool UserRolesModel::errored() { return d->errored; }
bool UserRolesModel::editable() { return d->editable; }

int UserRolesModel::rowCount(const QModelIndex& parent) const
{
	return d->userRoles.count();
}

enum Roles
{
	Name = Qt::UserRole,
	Color,
	Index,
};

QVariant UserRolesModel::data(const QModelIndex& idx, int role) const
{
	if (!checkIndex(idx)) {
		return QVariant();
	}

	auto id = d->userRoles[idx.row()];

	switch (role) {
	case Roles::Name:
		return d->guildRoles[id].name;
	case Roles::Color:
		return d->guildRoles[id].color;
	case Roles::Index:
		return idx;
	}

	return QVariant();
}

QHash<int,QByteArray> UserRolesModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret.insert(Roles::Name, QByteArray("name"));
	ret.insert(Roles::Color, QByteArray("colour"));
	ret.insert(Roles::Index, QByteArray("modelIndex"));

	return ret;
}

QVariantList UserRolesModel::guildRoles()
{
	QVariantList roles;

	for (auto role : d->guildRoles) {
		QVariantMap data;

		data["name"] = role.name;
		data["color"] = role.color;
		data["id"] = QString::number(role.id);

		roles << data;
	}

	return roles;
}

void UserRolesModel::remove(const QModelIndex& idx)
{
	if (!checkIndex(idx)) {
		return;
	}

	QtConcurrent::run([this, idx] {
		{
			protocol::chat::v1::ManageUserRolesRequest req;
			req.set_guild_id(d->guildID);
			req.set_user_id(d->userID);
			req.add_take_role_ids(d->userRoles[idx.row()]);

			if (!resultOk(d->client->chatKit->ManageUserRoles(req, theHeaders))) {
				return;
			}

			runOnMainThread([this, idx] {
				beginRemoveRows(QModelIndex(), idx.row(), idx.row());
				d->userRoles.removeAt(idx.row());
				endRemoveRows();
			});
		}
	});
}

void UserRolesModel::add(const QString &role)
{
	auto id = role.toULongLong();

	QtConcurrent::run([this, id] {
		{
			protocol::chat::v1::ManageUserRolesRequest req;
			req.set_guild_id(d->guildID);
			req.set_user_id(d->userID);
			req.add_give_role_ids(id);

			if (!resultOk(d->client->chatKit->ManageUserRoles(req, theHeaders))) {
				return;
			}

			runOnMainThread([this, id] {
				beginInsertRows(QModelIndex(), d->userRoles.length(), d->userRoles.length());
				d->userRoles << id;
				endInsertRows();
			});
		}
	});
}
