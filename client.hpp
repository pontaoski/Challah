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

class Client : public QObject
{
	Q_OBJECT

	static Client* mainClient;
	static QMap<QString,Client*> clients;

	std::string userToken;

	QString homeserver;

public:
	std::shared_ptr<grpc_impl::Channel> client;
	std::unique_ptr<protocol::core::v1::CoreService::Stub> coreKit;
	std::unique_ptr<protocol::foundation::v1::FoundationService::Stub> foundationKit;
	std::unique_ptr<protocol::profile::v1::ProfileService::Stub> profileKit;
	void authenticate(grpc::ClientContext& ctx);

private:
	void federateOtherClient(Client* client, const QString& target);

	Client();

public:
	quint64 userID;
	static Client* mainInstance();
	static Client* instanceForHomeserver(const QString& homeserver);
	bool login(const QString& email, const QString& password, const QString& homeserver);
	bool createGuild(const QString& name);
	GuildRepl guildInfo(quint64 id);
	void refreshGuilds();
};