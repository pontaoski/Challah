#include "state.h"
#include "members_model_p.h"

enum Roles {
	ID,
};

MembersModel::MembersModel(SDK::Client* client, quint64 guildID, State* state) : QAbstractListModel(state), d(new Private), s(state), c(client)
{
	d->gid = guildID;

	auto req = protocol::chat::v1::GetGuildMembersRequest{};
	req.set_guild_id(guildID);

	c->chatKit()->GetGuildMembers([this](auto r) {
		if (!resultOk(r)) {
			return;
		}

		protocol::chat::v1::GetGuildMembersResponse it = unwrap(r);
		beginResetModel();
		for (const auto& c : it.members()) {
			d->id << c;
		}
		endResetModel();
	}, req);
}

MembersModel::~MembersModel()
{
}

int MembersModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->id.length();
}

QVariant MembersModel::data(const QModelIndex& index, int role) const
{
	auto idx = index.row();
	if (idx >= d->id.length()) {
		return QVariant();
	}

	switch (role) {
	case Roles::ID:
		return QString::number(d->id[idx]);
	}

	return QVariant();
}

QHash<int,QByteArray> MembersModel::roleNames() const
{
	return {
		{ ID, "userID" }
	};
}
