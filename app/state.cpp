// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QSettings>
#include <QJSEngine>
#include <QQuickTextDocument>

#include "messages.hpp"
#include "richtexter.hpp"
#include "state.hpp"
#include "channels.hpp"
#include "userroles.hpp"

State* State::s_instance;

State::State()
{
	s_instance = this;
	client = Client::mainInstance();
	guildModel = new GuildModel;
}
State::~State()
{
	delete client;
	delete guildModel;
}
State* State::instance()
{
	return s_instance;
}
void State::logOut()
{
	delete Client::mainClient;
	auto copy = Client::mainClient;
	copy->stopEvents();
	Client::mainClient = nullptr;
	client = nullptr;

	delete guildModel;

	for (auto cli : Client::clients) {
		if (cli != copy) {
			cli->stopEvents();
			delete cli;
		}
	}
	Client::clients.clear();

	for (auto channelsModel : ChannelsModel::instances) {
		delete channelsModel;
	}
	ChannelsModel::instances.clear();

	this->guildModel = new GuildModel;
	this->client = Client::mainInstance();

	guildModelChanged();

	Q_EMIT loggedOut();
}
UserRolesModel* State::userRoles(const QString &userID, const QString &guildID, const QString &homeserver)
{
	return new UserRolesModel(userID.toULongLong(), guildID.toULongLong(), homeserver, nullptr);
}
bool State::startupLogin()
{
	QSettings settings;
	QVariant token = settings.value("state/token");
	QVariant hs = settings.value("state/homeserver");
	QVariant userID = settings.value("state/userid");
	if (token.isValid() && hs.isValid() && userID.isValid()) {
		if (client->consumeToken(token.toString(), userID.value<quint64>(), hs.toString())) {
			return true;
		}
	}

	return false;
}
ChannelsModel* State::channelsModel(const QString& guildID, const QString& homeserver)
{
	return guildModel->channelsModel(guildID.toULongLong(), homeserver);
}
MessagesModel* State::messagesModel(const QString& guildID, const QString& channelID, const QString& homeserver)
{
	return channelsModel(guildID, homeserver)->messagesModel(channelID.toULongLong());
}
bool State::createGuild(const QString &name)
{
	return client->createGuild(name);
}
bool State::joinGuild(const QString &inviteLink)
{
	auto str = inviteLink;
	str.remove(0, 10);
	auto split = str.split("/");
	if (split.length() != 2) {
		return false;
	}
	auto homeserver = split[0];
	auto invite = split[1];

	auto client = Client::instanceForHomeserver(homeserver);
	return client->joinInvite(invite);
}
bool State::leaveGuild(const QString &homeserver, const QString &id, bool isOwner)
{
	auto actualID = id.toULongLong();

	return Client::instanceForHomeserver(homeserver)->leaveGuild(actualID, isOwner);
}

void State::customEvent(QEvent *event)
{
	switch (event->type()) {
	case PleaseCallEvent::typeID: {
		auto ev = reinterpret_cast<PleaseCallEvent*>(event);

		QList<QJSValue> data;

		for (auto arg : ev->data.args) {
			data << ev->data.func.engine()->toScriptValue(arg);
		}

		ev->data.func.call(data);
	}
	}
}

void State::bindTextDocument(QQuickTextDocument* doc, const QString& homeserver, QObject* field)
{
	new TextFormatter(doc->textDocument(), homeserver, field);
}

void callJS(QJSValue func, QList<QVariant> args)
{
	auto call = PleaseCall{func, args};
	QCoreApplication::postEvent(State::instance(), new PleaseCallEvent(call));
}
