#include "state.h"
#include "overrides_model_p.h"

enum RoleNames
{
	Username,
	Avatar,
	Tags,
	Data,
};

OverridesModel::OverridesModel(QObject* parent, State* state) : QAbstractListModel(parent), d(new Private), s(state)
{
	protocol::profile::v1::GetAppDataRequest req;
	req.set_app_id("h.overrides");
	state->api()->mainClient()->GetAppData(req).then([this](auto r) {
		if (!r.ok()) {
			return;
		}

		protocol::profile::v1::GetAppDataResponse wrap = unwrap(r);
		protocol::profile::v1::AppDataOverrides overrides;
		if (!overrides.ParseFromString(wrap.app_data())) {
			return;
		}

		beginResetModel();
		for (auto& ov : overrides.overrides()) {
			d->overrides << ov;
		}
		endResetModel();
		Q_EMIT countChanged();
	});
}

OverridesModel::~OverridesModel()
{

}

int OverridesModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return d->overrides.length();
}

QVariant OverridesModel::data(const QModelIndex& index, int role) const
{
	const auto row = index.row();
	if (row >= d->overrides.length()) {
		return QVariant();
	}
	const auto& data = d->overrides[row];

	switch (role) {
	case Username:
		return data.has_username() ? QString::fromStdString(data.username()) : QString();
	case Avatar:
		return data.has_avatar() ? QString::fromStdString(data.avatar()) : QString();
	case Tags: {
		QVariantList tags;
		for (const auto& tag : data.tags()) {
			tags << QVariant::fromValue(Tag(tag.before(), tag.after()));
		}
		return tags;
	}
	case Data: {
		const auto username = QString::fromStdString(data.username());
		const auto avatar = QString::fromStdString(data.avatar());

		return QVariant::fromValue(OverrideData(username, avatar));
	}
	}

	return QVariant();
}

bool OverridesModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
	const auto row = idx.row();
	if (row >= d->overrides.length()) {
		return false;
	}
	auto& data = d->overrides[row];

	switch (role) {
	case Username:
		data.set_username(value.toString().toStdString());
	case Avatar:
		data.set_avatar(value.toString().toStdString());
	default:
		return false;
	}
	dataChanged(index(row), index(row));

	d->dirty = true;
	Q_EMIT dirtyChanged();

	return true;
}

void OverridesModel::addOverride(const QString& name, const QString& avatar, const QString& before, const QString& after)
{
	using namespace protocol::profile::v1;
	using namespace protocol::harmonytypes::v1;
	ProfileOverride over;

	over.set_allocated_system_plurality(new Empty);
	auto* bf = over.add_tags();
	bf->set_before(before.toStdString());
	bf->set_after(after.toStdString());
	over.set_username(name.toStdString());
	over.set_avatar(avatar.toStdString());

	d->dirty = true;
	Q_EMIT dirtyChanged();

	beginInsertRows(QModelIndex(), d->overrides.length(), d->overrides.length());
	d->overrides << over;
	endInsertRows();
	Q_EMIT countChanged();
}

QHash<int,QByteArray> OverridesModel::roleNames() const
{
	return {
		{ Username, "username", },
		{ Avatar, "avatar", },
		{ Tags, "tags", },
		{ Data, "overrideData", },
	};
}

bool OverridesModel::dirty() const
{
	return d->dirty;
}

void OverridesModel::save()
{
	using namespace protocol::profile::v1;
	using namespace protocol::harmonytypes::v1;

	AppDataOverrides overrides;

	google::protobuf::RepeatedPtrField<ProfileOverride> data
		(d->overrides.begin(), d->overrides.end());

	overrides.mutable_overrides()->Swap(&data);

	SetAppDataRequest req;
	req.set_app_id("h.overrides");
	req.set_app_data(overrides.SerializeAsString());

	s->api()->mainClient()->SetAppData(req).then([this](auto r) {
		if (r.ok()) {
			d->dirty = false;
			Q_EMIT dirtyChanged();
		}
	});
}

int OverridesModel::count() const
{
	return d->overrides.count();
}

OverrideData::OverrideData()
{

}

OverrideData::~OverrideData()
{

}
