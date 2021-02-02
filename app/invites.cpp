#include "invites.hpp"
#include "util.hpp"

#define theHeaders {{"Authorization", client->userToken}}

InviteModel::InviteModel(ChannelsModel *parent, QString homeServer, quint64 guildID) : QAbstractListModel(parent), homeserver(homeServer), guildID(guildID)
{
	client = Client::instanceForHomeserver(homeServer);

	protocol::chat::v1::GetGuildInvitesRequest req;
	req.set_guild_id(guildID);

	auto result = client->chatKit->GetGuildInvites(req, theHeaders);
	if (!resultOk(result)) {
		return;
	}
	auto resp = unwrap(result);

	for (auto invite : resp.invites()) {
		invites << Invite {
			.id = QString::fromStdString(invite.invite_id()),
			.possibleUses = invite.possible_uses(),
			.useCount = invite.use_count()
		};
	}
}

int InviteModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return invites.count();
}

QVariant InviteModel::data(const QModelIndex& index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	auto idx = index.row();

	switch (role) {
	case ID:
		return invites[idx].id;
	case PossibleUses:
		return invites[idx].possibleUses;
	case Uses:
		return invites[idx].useCount;
	}

	return QVariant();
}

QHash<int,QByteArray> InviteModel::roleNames() const
{
	QHash<int,QByteArray> ret;

	ret[ID] = "inviteID";
	ret[PossibleUses] = "possibleUses";
	ret[Uses] = "uses";

	return ret;
}


bool InviteModel::createInvite(const QString& id, qint32 possibleUses)
{
	protocol::chat::v1::CreateInviteRequest req;
	req.set_guild_id(guildID);
	req.set_name(id.toStdString());
	req.set_possible_uses(possibleUses);

	auto result = client->chatKit->CreateInvite(req, theHeaders);
	if (!resultOk(result)) {
		return false;
	}
	auto resp = unwrap(result);

	beginInsertRows(QModelIndex(), invites.length(), invites.length());
	invites << Invite {
		.id = QString::fromStdString(resp.name()),
		.possibleUses = possibleUses,
		.useCount = 0
	};
	endInsertRows();

	return true;
}

bool InviteModel::deleteInvite(const QString& id)
{
	protocol::chat::v1::DeleteInviteRequest req;
	req.set_guild_id(guildID);
	req.set_invite_id(id.toStdString());

	if (!resultOk(client->chatKit->DeleteInvite(req, theHeaders))) {
		return false;
	}

	auto idx = (std::find_if(invites.constBegin(), invites.constEnd(), [=](const Invite& inv) { return inv.id == id; }) - invites.begin());

	beginRemoveRows(QModelIndex(), idx, idx);
	invites.removeAt(idx);
	endRemoveRows();

	return true;
}
