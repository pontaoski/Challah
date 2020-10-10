#include "invites.hpp"

using grpc::ClientContext;

InviteModel::InviteModel(ChannelsModel *parent, QString homeServer, quint64 guildID) : QAbstractListModel(), homeserver(homeServer), guildID(guildID)
{
	client = Client::instanceForHomeserver(homeServer);

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::GetGuildInvitesRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID
	});
	protocol::core::v1::GetGuildInvitesResponse resp;

	if (checkStatus(client->coreKit->GetGuildInvites(&ctx, req, &resp))) {
		for (auto invite : resp.invites()) {
			invites << Invite {
				.id = QString::fromStdString(invite.invite_id()),
				.possibleUses = invite.possible_uses(),
				.useCount = invite.use_count()
			};
		}
	}
}

int InviteModel::rowCount(const QModelIndex& parent) const
{
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
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::CreateInviteRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID
	});
	req.set_name(id.toStdString());
	req.set_possible_uses(possibleUses);
	protocol::core::v1::CreateInviteResponse resp;

	if (!checkStatus(client->coreKit->CreateInvite(&ctx, req, &resp))) {
		return false;
	}

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
	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::DeleteInviteRequest req;
	req.set_allocated_location(Location {
		.guildID = guildID
	});
	req.set_invite_id(id.toStdString());
	google::protobuf::Empty resp;

	if (!checkStatus(client->coreKit->DeleteInvite(&ctx, req, &resp))) {
		return false;
	}

	auto idx = (std::find_if(invites.constBegin(), invites.constEnd(), [=](const Invite& inv) { return inv.id == id; }) - invites.begin());

	beginRemoveRows(QModelIndex(), idx, idx);
	invites.removeAt(idx);
	endRemoveRows();

	return true;
}
