#pragma once

#include <QDebug>
#include <QObject>
#include "client.hpp"

class State : public QObject
{
	Q_OBJECT

	GuildModel* guildModel;
	Client* client;

	friend class GuildModel;
	friend class Client;

	static State* s_instance;

public:
	State();
	~State();

	static State* instance();

	Q_INVOKABLE bool login(const QString& email, const QString& password, const QString &homeserver);
	Q_INVOKABLE bool createGuild(const QString& name);
	Q_INVOKABLE bool joinGuild(const QString& inviteLink);
	Q_PROPERTY(GuildModel* guildModel READ getGuildModel CONSTANT)
	GuildModel* getGuildModel() const { return guildModel; }
};

