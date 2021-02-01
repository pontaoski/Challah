#include "permissions.hpp"

#include "client.hpp"
#include "util.hpp"

#include "protos.hpp"

#include <QJSEngine>

#define theHeaders {{"Authorization", client->userToken}}

enum Roles
{
	NodeName = Qt::UserRole,
	Enabled
};

struct PermissionsModel::Private
{
	QList<protocol::chat::v1::Permission> perms;
};

using grpc::ClientContext;

PermissionsModel::PermissionsModel(QString homeserver, quint64 guildID, quint64 roleID) : QAbstractListModel(), homeserver(homeserver), guildID(guildID), roleID(roleID)
{
	client = Client::instanceForHomeserver(homeserver);
	d = new Private;

	protocol::chat::v1::GetPermissionsRequest req;
	req.set_guild_id(guildID);
	req.set_role_id(roleID);

	auto result = client->chatKit->GetPermissions(req, theHeaders);
	if (!resultOk(result)) {
		qWarning("TODO: implement error handling");
		isDirty = false;
		return;
	}
	auto resp = unwrap(result);

	auto perms = resp.perms().permissions();
	for (auto perm : perms) {
		d->perms << perm;
	}

	isDirty = false;
}

PermissionsModel::~PermissionsModel()
{
	delete d;
}


int PermissionsModel::rowCount(const QModelIndex& parent) const
{
	return d->perms.count();
}

void PermissionsModel::customEvent(QEvent *ev)
{
	if (ev->type() == PleaseCallEvent::typeID) {
		auto event = static_cast<PleaseCallEvent*>(ev);

		QList<QJSValue> data;

		for (auto arg : event->data.args) {
			data << event->data.func.engine()->toScriptValue(arg);
		}

		event->data.func.call(data);
	}
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

void PermissionsModel::save()
{
	protocol::chat::v1::SetPermissionsRequest req;
	req.set_guild_id(guildID);
	req.set_role_id(roleID);

	auto list = new protocol::chat::v1::PermissionList;
	list->add_permissions();
	for (auto perm : d->perms) {
		*(list->mutable_permissions()->Add()) = perm;
	}
	req.set_allocated_perms(list);

	if (resultOk(client->chatKit->SetPermissions(req, theHeaders))) {
		isDirty = false;
		Q_EMIT isDirtyChanged();
	}
}

void PermissionsModel::deletePermission(int idx)
{
	beginRemoveRows(QModelIndex(), idx, idx);

	d->perms.removeAt(idx);
	isDirty = true;
	Q_EMIT isDirtyChanged();

	endRemoveRows();
}
