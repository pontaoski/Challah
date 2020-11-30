#include "roles.hpp"

#include "core.grpc.pb.h"
#include "core.pb.h"

#include "client.hpp"
#include "util.hpp"

#include <QColor>

#define doContext(c) ClientContext c; client->authenticate(c)

using grpc::ClientContext;

struct RolesModel::Private
{
	QList<protocol::core::v1::Role> roles;
};

RolesModel::RolesModel(QString homeserver, quint64 guildID) : QAbstractListModel(), homeServer(homeserver), guildID(guildID)
{
	client = Client::instanceForHomeserver(homeServer);
	d = new Private;

	doContext(ctx);

	protocol::core::v1::GetGuildRolesRequest req;
	req.set_guild_id(guildID);
	protocol::core::v1::GetGuildRolesResponse resp;

	checkStatus(client->coreKit->GetGuildRoles(&ctx, req, &resp));

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

	doContext(ctx);

	protocol::core::v1::MoveRoleRequest req;
	req.set_guild_id(guildID);
	req.set_role_id(fromRole.role_id());
	protocol::core::v1::MoveRoleResponse resp;

	if (to == 0) {
		req.set_before_id(d->roles[0].role_id());
	} else if (to + 1 == d->roles.length()) {
		req.set_after_id(d->roles[to].role_id());
	} else {
		req.set_after_id(d->roles[to-1].role_id());
		req.set_before_id(d->roles[to+1].role_id());
	}

	checkStatus(client->coreKit->MoveRole(&ctx, req, &resp));
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
	}

	return QVariant();
}

bool RolesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!checkIndex(index))
		return false;

	switch (role)
	{
	case NameRole:
		{
			doContext(ctx);

			protocol::core::v1::ModifyGuildRoleRequest req;
			req.set_guild_id(guildID);
			req.set_allocated_role(new protocol::core::v1::Role);
			req.set_modify_name(true);
			req.mutable_role()->set_name(value.toString().toStdString());
			google::protobuf::Empty empty;

			return checkStatus(client->coreKit->ModifyGuildRole(&ctx, req, &empty));
		}
	}

	return false;
}

bool RolesModel::createRole(const QString& name, const QColor& colour)
{
	doContext(ctx);

	protocol::core::v1::AddGuildRoleRequest req;
	req.set_guild_id(guildID);
	req.set_allocated_role(new protocol::core::v1::Role);
	req.mutable_role()->set_name(name.toStdString());
	req.mutable_role()->set_color(colour.rgba());

	protocol::core::v1::AddGuildRoleResponse resp;

	if (!checkStatus(client->coreKit->AddGuildRole(&ctx, req, &resp))) {
		return false;
	}

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

	return ret;
}
