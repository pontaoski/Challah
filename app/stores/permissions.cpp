#include "permissions.h"

#include "chat/v1/permissions.pb.h"
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

PermissionsModel::PermissionsModel(QString host, quint64 guildID, quint64 roleID, State* state) : QAbstractListModel(state), d(new Private), s(state), host(host)
{
	d->guildID = guildID;
	d->roleID = roleID;

	protocol::chat::v1::GetPermissionsRequest req;
	req.set_guild_id(guildID);
	req.set_role_id(roleID);

	s->api()->dispatch(host, &SDK::R::GetPermissions, req).then([this](Result<protocol::chat::v1::GetPermissionsResponse, QString> result) {
		if (!result.ok()) {
			qWarning("TODO: implement error handling");
			isDirty = false;
			return;
		}

		auto resp = result.value();
		auto perms = resp.perms();
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
		return d->perms[index.row()].ok();
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

		d->perms[index.row()].set_ok(value.toBool());
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
	perm.set_ok(allow);

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

	req.add_perms_to_give();
	for (auto perm : d->perms) {
		*(req.mutable_perms_to_give()->Add()) = perm;
	}

	auto res = co_await s->api()->dispatch(host, &SDK::R::SetPermissions, req);
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
