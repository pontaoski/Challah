#include "chat/v1/guilds.pb.h"
#include "invites_p.h"
#include "coroutine_integration.h"

enum Roles
{
	Name,
	PossibleUses,
	Uses,
};

InviteModel::InviteModel(SDK::Client* client, quint64 guildID, State* state) : QAbstractListModel(client), d(new Private), s(state), c(client)
{
	d->guildID = guildID;

	protocol::chat::v1::GetGuildInvitesRequest req;
	req.set_guild_id(guildID);

	c->chatKit()->GetGuildInvites(req).then([this](Result<protocol::chat::v1::GetGuildInvitesResponse, QString> r) {
		if (!r.ok()) {
			return;
		}
		auto resp = r.value();

		beginResetModel();
		for (const auto& invite : resp.invites()) {
			d->invites << Invite {
				QString::fromStdString(invite.invite_id()),
				qint32(invite.invite().possible_uses()),
				qint32(invite.invite().use_count())
			};
		}
		endResetModel();
	});
}

InviteModel::~InviteModel()
{
}

int InviteModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->invites.count();
}

QVariant InviteModel::data(const QModelIndex& index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	auto idx = index.row();

	switch (role) {
	case Name:
		return d->invites[idx].id;
	case PossibleUses:
		return d->invites[idx].possibleUses;
	case Uses:
		return d->invites[idx].useCount;
	}

	return QVariant();
}

QHash<int,QByteArray> InviteModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[Name] = "name";
	ret[PossibleUses] = "possibleUses";
	ret[Uses] = "uses";

	return ret;
}


FutureBase InviteModel::createInvite(QString id, qint32 possibleUses)
{
	protocol::chat::v1::CreateInviteRequest req;
	req.set_guild_id(d->guildID);
	req.set_name(id.toStdString());
	req.set_possible_uses(possibleUses);

	auto reply = co_await c->chatKit()->CreateInvite(req);

	if (!reply.ok()) {
		co_return false;
	}

	auto resp = reply.value();

	beginInsertRows(QModelIndex(), d->invites.length(), d->invites.length());
	d->invites << Invite {
		QString::fromStdString(resp.invite_id()),
		possibleUses,
		0
	};
	endInsertRows();

	co_return true;
}

FutureBase InviteModel::deleteInvite(QString id)
{
	protocol::chat::v1::DeleteInviteRequest req;
	req.set_guild_id(d->guildID);
	req.set_invite_id(id.toStdString());

	auto reply = co_await c->chatKit()->DeleteInvite(req);

	if (!reply.ok()) {
		co_return false;
	}

	auto idx = (std::find_if(d->invites.constBegin(), d->invites.constEnd(), [=](const Invite& inv) { return inv.id == id; }) - d->invites.begin());

	beginRemoveRows(QModelIndex(), idx, idx);
	d->invites.removeAt(idx);
	endRemoveRows();

	co_return true;
}
