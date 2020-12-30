// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QList>

#include "grpc++/grpc++.h"

#include "chat/v1/chat.grpc.pb.h"
#include "chat/v1/chat.pb.h"

#include "auth/v1/auth.grpc.pb.h"
#include "auth/v1/auth.pb.h"

#include "mediaproxy/v1/mediaproxy.grpc.pb.h"
#include "mediaproxy/v1/mediaproxy.pb.h"

#include "guild.hpp"
#include "util.hpp"

class State;

class Client : public QObject
{
	Q_OBJECT

	static Client* mainClient;
	static QMap<QString,Client*> clients;

	QString homeserver;
	std::unique_ptr<grpc::ClientReaderWriterInterface<protocol::chat::v1::StreamEventsRequest,protocol::chat::v1::Event>> eventStream;

	friend class State;
	friend class LoginManager;

public:
	std::string userToken;
	std::shared_ptr<grpc::Channel> client;
	std::unique_ptr<protocol::chat::v1::ChatService::Stub> chatKit;
	std::unique_ptr<protocol::auth::v1::AuthService::Stub> authKit;
	std::unique_ptr<protocol::mediaproxy::v1::MediaProxyService::Stub> mediaProxyKit;
	void authenticate(grpc::ClientContext& ctx);

private:
	void federateOtherClient(Client* client, const QString& target);

	Client();
	void runEvents();
	bool forgeNewConnection();

public:
	quint64 userID;
	static Client* mainInstance();
	static Client* instanceForHomeserver(const QString& homeserver);
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
