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