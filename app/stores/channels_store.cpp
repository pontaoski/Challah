#include "state.h"
#include "channels_p.h"

enum Roles {
	Name,
	Kind,
};

ChannelsStore::ChannelsStore(State* state, ChannelsModel* parent) : ChallahAbstractRelationalModel(parent), d(new Private), s(state)
{
	d->cm = parent;
}

ChannelsStore::~ChannelsStore()
{

}

QVariant ChannelsStore::data(const QVariant& key, int role)
{
	if (!checkKey(key)) {
		return QVariant();
	}

	auto id = key.toString().toULongLong();

	namespace chat = protocol::chat::v1;

	switch (role) {
	case Roles::Name:
		return QString::fromStdString(d->data[id].channel().channel_name());
	case Roles::Kind: {
		switch (d->data[id].channel().kind()) {
		case chat::CHANNEL_KIND_VOICE_MEDIA:
			return "voice";
		default:
			return "text";
		}
	}
	}

	return QVariant();
}

bool ChannelsStore::checkKey(const QVariant& key)
{
	auto id = key.toString().toULongLong();

	return d->data.contains(id);
}

QHash<int,QByteArray> ChannelsStore::roleNames()
{
	return {
		{ Name, "name" },
		{ Kind, "kind" },
	};
}

void ChannelsStore::setChannelName(const QString &id, const QString &name)
{
	auto req = protocol::chat::v1::UpdateChannelInformationRequest { };
	req.set_guild_id(d->cm->d->gid);
	req.set_channel_id(id.toULongLong());
	req.set_new_name(name.toStdString());
	s->api()->dispatch(d->cm->d->host, &SDK::R::UpdateChannelInformation, req);
}
