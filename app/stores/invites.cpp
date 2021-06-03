#include "invites_p.h"

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

	c->chatKit()->GetGuildInvites([this](auto r) {
		if (!resultOk(r)) {
			return;
		}
		auto resp = unwrap(r);
		beginResetModel();
		for (const auto& invite : resp.invites()) {
			d->invites << Invite {
				QString::fromStdString(invite.invite_id()),
				invite.possible_uses(),
				invite.use_count()
			};
		}
		endResetModel();
	}, req);
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


QIviPendingReply<bool> InviteModel::createInvite(const QString& id, qint32 possibleUses)
{
	QIviPendingReply<bool> reply;

	protocol::chat::v1::CreateInviteRequest req;
	req.set_guild_id(d->guildID);
	req.set_name(id.toStdString());
	req.set_possible_uses(possibleUses);

	c->chatKit()->CreateInvite([this, reply, possibleUses](auto r) {
		auto repl = reply;
		if (!resultOk(r)) {
			repl.setSuccess(false);
			return;
		}

		auto resp = unwrap(r);

		beginInsertRows(QModelIndex(), d->invites.length(), d->invites.length());
		d->invites << Invite {
			QString::fromStdString(resp.name()),
			possibleUses,
			0
		};
		endInsertRows();

		repl.setSuccess(true);
	}, req);

	return reply;
}

QIviPendingReply<bool> InviteModel::deleteInvite(const QString& id)
{
	QIviPendingReply<bool> reply;

	protocol::chat::v1::DeleteInviteRequest req;
	req.set_guild_id(d->guildID);
	req.set_invite_id(id.toStdString());

	c->chatKit()->DeleteInvite([this, reply, id](auto r) {
		auto repl = reply;
		if (!resultOk(r)) {
			repl.setSuccess(false);
			return;
		}

		auto idx = (std::find_if(d->invites.constBegin(), d->invites.constEnd(), [=](const Invite& inv) { return inv.id == id; }) - d->invites.begin());

		beginRemoveRows(QModelIndex(), idx, idx);
		d->invites.removeAt(idx);
		endRemoveRows();
		repl.setSuccess(true);
	}, req);

	return reply;
}
