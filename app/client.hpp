// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QList>

#include "protos.hpp"

#include "guild.hpp"
#include "util.hpp"

class State;

class Client : public QObject
{
	Q_OBJECT

	static Client* mainClient;
	static QMap<QString,Client*> clients;

	std::unique_ptr<Receive__protocol_chat_v1_Event__Send__protocol_chat_v1_StreamEventsRequest__Stream> eventStream;

	friend class State;
	friend class LoginManager;

public:
	QString homeserver;
	QString userToken;
	std::unique_ptr<ChatServiceServiceClient> chatKit;
	std::unique_ptr<AuthServiceServiceClient> authKit;
	std::unique_ptr<MediaProxyServiceServiceClient> mediaProxyKit;
	QSet<quint64> subscribedGuilds;
	bool shouldRestartStreams = true;

private:
	void federateOtherClient(Client* client, const QString& target);

	Client();
	void runEvents();
	bool forgeNewConnection();

protected:
	void customEvent(QEvent *event) override;

public:
	quint64 userID;
	static Client* mainInstance();
	static Client* instanceForHomeserver(const QString& homeserver);
	void stopEvents();
	bool joinInvite(const QString& invite);
	void consumeSession(protocol::auth::v1::Session session, const QString& homeserver);
	void subscribeGuild(quint64 guild);
	bool createGuild(const QString& name);
	bool leaveGuild(quint64 id, bool isOwner);
	bool hasPermission(const QString& node, quint64 guildID, quint64 channelID = 0);
	GuildRepl guildInfo(quint64 id);
	bool consumeToken(const QString& token, quint64 userID, const QString& homeserver);
	bool refreshGuilds();
};
