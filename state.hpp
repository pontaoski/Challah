#pragma once

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
};

