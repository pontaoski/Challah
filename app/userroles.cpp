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

using grpc::ClientContext;
#define do(ctx) ClientContext ctx; d->client->authenticate(ctx)

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

	do(ctx);

	protocol::chat::v1::GetUserRolesRequest req;
	req.set_user_id(userID);
	req.set_guild_id(guildID);

	protocol::chat::v1::GetUserRolesResponse resp;

	if (!checkStatus(d->client->chatKit->GetUserRoles(&ctx, req, &resp))) {
		d->errored = true;
		return;
	}

	protocol::chat::v1::GetGuildRolesRequest req2;
	req2.set_guild_id(guildID);

	protocol::chat::v1::GetGuildRolesResponse resp2;
	do(ctx2);

	if (!checkStatus(d->client->chatKit->GetGuildRoles(&ctx2, req2, &resp2))) {
		d->errored = true;
		return;
	}

	do(ctx3);
	protocol::chat::v1::QueryPermissionsRequest req3;
	req3.set_guild_id(guildID);
	protocol::chat::v1::QueryPermissionsResponse resp3;

	if (!checkStatus(d->client->chatKit->QueryHasPermission(&ctx3, req3, &resp3))) {
		d->editable = false;
	} else {
		d->editable = resp3.ok();
	}

	for (auto guildrole : resp2.roles()) {
		d->guildRoles[guildrole.role_id()] = RoleData {
			.name = QString::fromStdString(guildrole.name()),
			.color = QColor::fromRgba(guildrole.color()),
			.hoist = guildrole.hoist(),
			.pingable = guildrole.pingable(),
			.id = guildrole.role_id(),
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
			do(ctx);

			protocol::chat::v1::ManageUserRolesRequest req;
			req.set_guild_id(d->guildID);
			req.set_user_id(d->userID);
			req.add_take_role_ids(d->userRoles[idx.row()]);
			google::protobuf::Empty resp;

			if (!checkStatus(d->client->chatKit->ManageUserRoles(&ctx, req, &resp))) {
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
			do(ctx);

			protocol::chat::v1::ManageUserRolesRequest req;
			req.set_guild_id(d->guildID);
			req.set_user_id(d->userID);
			req.add_give_role_ids(id);
			google::protobuf::Empty resp;

			if (!checkStatus(d->client->chatKit->ManageUserRoles(&ctx, req, &resp))) {
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
