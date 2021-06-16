#include "permissions.h"

#include "client.h"
#include "state.h"

#include <QJSEngine>

enum Roles
{
	NodeName = Qt::UserRole,
	Enabled
};

struct PermissionsModel::Private
{
	quint64 roleID;
	quint64 guildID;
	QList<protocol::chat::v1::Permission> perms;
};

PermissionsModel::PermissionsModel(SDK::Client* client, quint64 guildID, quint64 roleID, State* state) : QAbstractListModel(client), d(new Private), s(state), c(client)
{
	d->guildID = guildID;
	d->roleID = roleID;

	protocol::chat::v1::GetPermissionsRequest req;
	req.set_guild_id(guildID);
	req.set_role_id(roleID);

	c->chatKit()->GetPermissions(req).then([this](auto result) {
		if (!result.ok()) {
			qWarning("TODO: implement error handling");
			isDirty = false;
			return;
		}

		auto resp = result.value();
		auto perms = resp.perms().permissions();
		for (auto perm : perms) {
			d->perms << perm;
		}

		isDirty = false;
	});
}

PermissionsModel::~PermissionsModel()
{
}


int PermissionsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->perms.count();
}

QVariant PermissionsModel::data(const QModelIndex& index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case NodeName:
		return QString::fromStdString(d->perms[index.row()].matches());
	case Enabled:
		return d->perms[index.row()].mode() == protocol::chat::v1::Permission_Mode_Allow;
	}

	return QVariant();
}

bool PermissionsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!checkIndex(index))
		return false;

	switch (role)
	{
	case Enabled:
		if (value.type() != QVariant::Bool) {
			return false;
		}

		isDirty = true;
		Q_EMIT isDirtyChanged();

		d->perms[index.row()].set_mode(value.toBool() ? protocol::chat::v1::Permission_Mode_Allow : protocol::chat::v1::Permission_Mode_Deny);
		return true;
	}

	return false;
}

QHash<int,QByteArray> PermissionsModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[NodeName] = "nodeName";
	ret[Enabled] = "enabled";

	return ret;
}

void PermissionsModel::addPermission(const QString& node, bool allow)
{
	beginInsertRows(QModelIndex(), d->perms.length(), d->perms.length());

	protocol::chat::v1::Permission perm;
	perm.set_matches(node.toStdString());
	perm.set_mode(allow ? protocol::chat::v1::Permission_Mode_Allow : protocol::chat::v1::Permission_Mode_Deny);

	d->perms << perm;

	isDirty = true;
	Q_EMIT isDirtyChanged();

	endInsertRows();
}

FutureBase PermissionsModel::save()
{
	protocol::chat::v1::SetPermissionsRequest req;
	req.set_guild_id(d->guildID);
	req.set_role_id(d->roleID);

	auto list = new protocol::chat::v1::PermissionList;
	list->add_permissions();
	for (auto perm : d->perms) {
		*(list->mutable_permissions()->Add()) = perm;
	}
	req.set_allocated_perms(list);

	auto res = co_await c->chatKit()->SetPermissions(req);
	if (res.ok()) {
		isDirty = false;
		Q_EMIT isDirtyChanged();
	}

	co_return res.ok();
}

void PermissionsModel::deletePermission(int idx)
{
	beginRemoveRows(QModelIndex(), idx, idx);

	d->perms.removeAt(idx);
	isDirty = true;
	Q_EMIT isDirtyChanged();

	endRemoveRows();
}
