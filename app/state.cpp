#include <QSettings>
#include <QQmlEngine>

#include "state_p.h"

State::State(QQmlEngine* object) : QObject(object), d(new Private)
{
	d->eng = object;
	createComponents();

	connect(d->sdk.get(), &SDK::ClientManager::authEvent, this, &State::handleStep);
	connect(d->sdk.get(), &SDK::ClientManager::ready, this, &State::endLogin);
	connect(d->sdk.get(), &SDK::ClientManager::ready, this, [](const QString& hs, quint64 userID, const QString& tok) {
		QSettings settings;

		settings.setValue("state/homeserver", hs);
		settings.setValue("state/userid", userID);
		settings.setValue("state/token", tok);
	});
	connect(this, &State::endLogin, this, [this]() {
		createModels();
	});
}

State::~State() {}

void State::doInitialLogin()
{
	QSettings settings;

	QVariant token = settings.value("state/token");
	QVariant hs = settings.value("state/homeserver");
	QVariant userID = settings.value("state/userid");

	if (token.isValid() && hs.isValid() && userID.isValid()) {
		d->sdk->checkLogin([this](bool ok) {
			if (!ok) {
				Q_EMIT beginHomeserver();
			}
		}, token.toString(), hs.toString(), userID.toULongLong());
	} else {
		Q_EMIT beginHomeserver();
	}
}

void State::doLogin(const QString& homeserver)
{
	d->sdk->beginAuthentication(homeserver);
	Q_EMIT beginLogin();
}

SDK::ClientManager* State::api()
{
	return d->sdk.get();
}

GuildList* State::guildList()
{
	return d->list;
}

GuildsStore* State::guildsStore()
{
	return d->store;
}

MembersStore* State::membersStore()
{
	return d->membersStore;
}

void State::createModels()
{
	d->list = new GuildList(this);
	Q_EMIT guildListChanged();

	d->store = new GuildsStore(this);
	Q_EMIT guildsStoreChanged();

	d->membersStore = new MembersStore(this);
}

ChannelsModel* State::channelsModelFor(const QString& host, const QString& guildID, QObject *it)
{
	auto eng = qmlEngine(it);

	auto c = d->sdk->clientForHomeserver(host);
	auto id = guildID.toULongLong();
	auto tup = qMakePair(c, id);

	if (d->channelsModels.contains(tup) and not d->channelsModels[tup].isNull()) {
		return d->channelsModels[tup];
	}

	auto mod = new ChannelsModel(c, id, this);
	eng->setObjectOwnership(mod, QQmlEngine::JavaScriptOwnership);
	d->channelsModels[tup] = mod;

	return mod;
}

MembersModel* State::membersModelFor(const QString& host, const QString& guildID, QObject *it)
{
	auto eng = qmlEngine(it);

	auto c = d->sdk->clientForHomeserver(host);
	auto id = guildID.toULongLong();
	auto tup = qMakePair(c, id);

	if (d->membersModels.contains(tup) and not d->membersModels[tup].isNull()) {
		return d->membersModels[tup];
	}

	auto mod = new MembersModel(c, id, this);
	eng->setObjectOwnership(mod, QQmlEngine::JavaScriptOwnership);
	d->membersModels[tup] = mod;

	return mod;
}

MessagesModel* State::messagesModelFor(const QString& host, const QString& guildID, const QString& channelID, QObject* it)
{
	auto eng = qmlEngine(it);

	auto c = d->sdk->clientForHomeserver(host);
	auto gid = guildID.toULongLong();
	auto cid = channelID.toULongLong();
	auto tup = qMakePair(c, qMakePair(gid, cid));

	if (d->messagesModels.contains(tup) and not d->messagesModels[tup].isNull()) {
		return d->messagesModels[tup];
	}

	auto mod = new MessagesModel(c, gid, cid, this);
	eng->setObjectOwnership(mod, QQmlEngine::JavaScriptOwnership);
	d->messagesModels[tup] = mod;

	return mod;
}
