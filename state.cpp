// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

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
bool State::login(const QString &email, const QString &password, const QString &homeserver)
{
	return client->login(email, password, homeserver);
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