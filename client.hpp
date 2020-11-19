// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QList>

#include "grpc++/grpc++.h"

#include "core.grpc.pb.h"
#include "core.pb.h"

#include "foundation.grpc.pb.h"
#include "foundation.pb.h"

#include "profile.grpc.pb.h"
#include "profile.pb.h"

#include "guild.hpp"
#include "util.hpp"

class State;

class Client : public QObject
{
	Q_OBJECT

	static Client* mainClient;
	static QMap<QString,Client*> clients;

	QString homeserver;
	std::unique_ptr<grpc::ClientReaderWriterInterface<protocol::core::v1::StreamEventsRequest,protocol::core::v1::Event>> eventStream;

	friend class State;

public:
	std::string userToken;
	std::shared_ptr<grpc_impl::Channel> client;
	std::unique_ptr<protocol::core::v1::CoreService::Stub> coreKit;
	std::unique_ptr<protocol::foundation::v1::FoundationService::Stub> foundationKit;
	std::unique_ptr<protocol::profile::v1::ProfileService::Stub> profileKit;
	void authenticate(grpc::ClientContext& ctx);

private:
	void federateOtherClient(Client* client, const QString& target);

	Client();
	void runEvents();

public:
	quint64 userID;
	static Client* mainInstance();
	static Client* instanceForHomeserver(const QString& homeserver);
	bool joinInvite(const QString& invite);
	bool login(const QString& email, const QString& password, const QString& homeserver);
	void subscribeGuild(quint64 guild)
	{
		protocol::core::v1::StreamEventsRequest req;
		auto subReq = new protocol::core::v1::StreamEventsRequest_SubscribeToGuild;
		auto loc = new protocol::core::v1::Location;
		loc->set_guild_id(guild);
		subReq->set_allocated_location(loc);
		req.set_allocated_subscribe_to_guild(subReq);

		eventStream->Write(req);
	}
	bool createGuild(const QString& name);
	bool leaveGuild(quint64 id, bool isOwner);
	GuildRepl guildInfo(quint64 id);
	bool consumeToken(const QString& token, quint64 userID, const QString& homeserver);
	bool refreshGuilds();
};
