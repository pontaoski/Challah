#include "state.h"
#include "members_model_p.h"

enum Roles {
	ID,
};

MembersModel::MembersModel(QString host, quint64 guildID, State* state) : QAbstractListModel(state), d(new Private), s(state), host(host)
{
	d->gid = guildID;

	auto req = protocol::chat::v1::GetGuildMembersRequest{};
	req.set_guild_id(guildID);

	if (guildID == 0) {
		return;
	}

	s->api()->dispatch(host, &SDK::R::GetGuildMembers, req).then([this](auto r) {
		if (!resultOk(r)) {
			return;
		}

		protocol::chat::v1::GetGuildMembersResponse it = unwrap(r);
		beginResetModel();
		for (const auto& c : it.members()) {
			d->id << c;
		}
		endResetModel();
	});

	state->api()->subscribeToGuild(host, guildID);
	connect(state->api(), &SDK::ClientManager::chatEvent, this, [=](QString hs, protocol::chat::v1::StreamEvent ev) {
		using namespace protocol::chat::v1;
		if (hs != host) {
			return;
		}

		switch (ev.event_case()) {
		case StreamEvent::kJoinedMember: {
			auto jm = ev.joined_member();
			if (jm.guild_id() != d->gid) {
				return;
			}

			beginInsertRows(QModelIndex(), d->id.length(), d->id.length());
			d->id << jm.member_id();
			endInsertRows();

			break;
		}
		case StreamEvent::kLeftMember: {
			auto lm = ev.left_member();
			if (lm.guild_id() != d->gid) {
				return;
			}

			auto idx = d->id.indexOf(lm.member_id());

			beginRemoveRows(QModelIndex(), idx, idx);
			d->id.removeAt(idx);
			endRemoveRows();

			break;
		}
		}
	});
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
