// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QSettings>
#include <QJSEngine>
#include <QQuickTextDocument>

#include "richtexter.hpp"
#include "state.hpp"

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
bool State::login(const QString &email, const QString &password, const QString &homeserver)
{
	if (client->login(email, password, homeserver)) {
		Q_EMIT loggedIn();
		return true;
	}
	return false;
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

void State::bindTextDocument(QQuickTextDocument* doc, QObject* field)
{
	new TextFormatter(doc->textDocument(), field);
}

void callJS(QJSValue func, QList<QVariant> args)
{
	auto call = PleaseCall{func, args};
	QCoreApplication::postEvent(State::instance(), new PleaseCallEvent(call));
}
