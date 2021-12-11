#include <QJsonArray>

#include "state.h"

#include "members_store_p.h"

#include "debounce.h"

enum Role {
	Name,
	AvatarURL,
};

MembersStore::MembersStore(State* state) : ChallahAbstractRelationalModel(state), d(new Private), s(state)
{
	d->debounced = QFunctionUtils::Debounce([this] { this->fetchBatched(); }, 10);
}

MembersStore::~MembersStore()
{

}

QPair<QString, quint64> from(const QVariant& hi)
{
	const auto js = hi.value<QVariantList>();
	return qMakePair(js[0].toString(), js[1].toString().toULongLong());
}

QVariant to(const QPair<QString, quint64>& v)
{
	QJsonArray it;
	it << v.first << QString::number(v.second);
	return it;
}

bool MembersStore::checkKey(const QVariant& key)
{
	auto it = from(key);
	return d->data.contains(it.first) && d->data[it.first].contains(it.second);
}

bool MembersStore::canFetchKey(const QVariant& key)
{
	Q_UNUSED(key);

	return true;
}

void MembersStore::fetchBatched()
{
	const auto items = d->toBatch;
	d->toBatch.clear();

	for (auto& hs : items.keys())
	{
		const auto& ids = items[hs];

		QList<protocol::profile::v1::GetProfileRequest> items;
		items.reserve(ids.size());

		for (quint64 id : ids) {
			protocol::profile::v1::GetProfileRequest req;
			req.set_user_id(id);

			items << req;
		}

		s->api()->dispatch(hs, &SDK::R::BatchGetProfile, items).then([this, hs = hs, ids = ids](Result<QList<protocol::profile::v1::GetProfileResponse>, QString> r) {
			if (!resultOk(r)) {
				return;
			}

			const auto res = unwrap(r);

			int i = 0;
			for (quint64 id : ids) {
				auto res = r.value();
				d->data[hs][id] = res[i].profile();
				Q_EMIT keyAdded(to(qMakePair(hs, id)));

				i++;
			}
		});
	}
}

void MembersStore::fetchKey(const QVariant& key)
{
	auto it = from(key);

	if (it.second == 0) return;

	d->toBatch[it.first] = d->toBatch.value(it.first, {});
	d->toBatch[it.first] << it.second;

	d->debounced();
}

QVariant MembersStore::data(const QVariant& key, int role)
{
	auto it = from(key);

	if (!checkKey(key)) {
		return QVariant();
	}

	switch (role) {
	case Role::Name:
		return QString::fromStdString(d->data[it.first][it.second].user_name());
	case Role::AvatarURL: {
		auto hmc = QString::fromStdString(d->data[it.first][it.second].user_avatar());
		if (hmc.isEmpty()) {
			return QString();
		}
		return s->mediaURL(hmc, s->api()->clientForHomeserver("local").result()->homeserver());
	}
	}

	return QVariant();
}

QHash<int,QByteArray> MembersStore::roleNames()
{
	return {
		{ Name,"name" },
		{ AvatarURL, "avatarURL" },
	};
}
