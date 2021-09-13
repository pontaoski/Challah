#include "channels_p.h"

enum Roles {
	Name,
};

ChannelsStore::ChannelsStore(State* state, ChannelsModel* parent) : ChallahAbstractRelationalModel(parent), d(new Private), s(state)
{

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

	switch (role) {
	case Roles::Name:
		return QString::fromStdString(d->data[id].channel().channel_name());
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
		{ Name, "name" }
	};
}
