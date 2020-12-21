// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QtConcurrent>
#include <QDebug>

#include "state.hpp"
#include "client.hpp"
#include "util.hpp"
#include "channels.hpp"
#include <unistd.h>

using grpc::ClientContext;

void Client::authenticate(grpc::ClientContext &ctx)
{
	ctx.AddMetadata("auth", userToken);
}

bool Client::refreshGuilds()
{
	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::core::v1::GetGuildListRequest{};
	protocol::core::v1::GetGuildListResponse resp;

	if (!checkStatus(coreKit->GetGuildList(&ctx, req, &resp))) {
		return false;
	}

	State::instance()->guildModel->guilds.clear();
	State::instance()->guildModel->beginResetModel();

	auto guilds = resp.guilds();
	for (auto guild : guilds)
	{
		auto data = Client::instanceForHomeserver(QString::fromStdString(guild.host()))->guildInfo(guild.guild_id());

		State::instance()->guildModel->guilds << Guild {
			.guildID = guild.guild_id(),
			.ownerID = data.ownerID,
			.homeserver = QString::fromStdString(guild.host()),
			.name = data.name,
			.picture = data.picture,
		};
	}

	State::instance()->guildModel->endResetModel();
	return true;
}

Client::Client() : QObject()
{
}

Client* Client::mainInstance()
{
	static bool madeClient = false;

	if (!madeClient) {
		madeClient = true;
		mainClient = new Client;
	}

	return mainClient;
}

Client* Client::instanceForHomeserver(const QString& homeserver)
{
	if (homeserver == "local") {
		return mainInstance();
	}

	if (!clients.contains(homeserver)) {
		auto client = new Client;
		clients[homeserver] = client;
		mainClient->federateOtherClient(client, homeserver);
	}

	return clients[homeserver];
}

GuildRepl Client::guildInfo(quint64 id)
{
	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::core::v1::GetGuildRequest {};
	req.set_guild_id(id);

	auto repl = protocol::core::v1::GetGuildResponse {};

	if (!checkStatus(coreKit->GetGuild(&ctx, req, &repl))) {
		return GuildRepl{};
	}

	return GuildRepl {
		.ownerID = repl.guild_owner(),
		.name = QString::fromStdString(repl.guild_name()),
		.picture = QString::fromStdString(repl.guild_picture()),
	};
}

void Client::federateOtherClient(Client* client, const QString& target)
{
	client->client = grpc::CreateChannel(target.toStdString(), grpc::SslCredentials(grpc::SslCredentialsOptions()));
	client->homeserver = target;
	client->coreKit = protocol::core::v1::CoreService::NewStub(client->client);
	client->foundationKit = protocol::foundation::v1::FoundationService::NewStub(client->client);
	client->profileKit = protocol::profile::v1::ProfileService::NewStub(client->client);

	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::foundation::v1::FederateRequest{};
	req.set_target(target.toStdString());
	auto repl = protocol::foundation::v1::FederateReply{};

	if (!checkStatus(foundationKit->Federate(&ctx, req, &repl))) {
		return;
	}

	ClientContext ctx2;

	auto req2 = protocol::foundation::v1::LoginRequest {};
	auto remote = new protocol::foundation::v1::LoginRequest_Federated {};
	remote->set_auth_token(repl.token());
	remote->set_domain(homeserver.toStdString());
	req2.set_allocated_federated(remote);

	auto repl2 = protocol::foundation::v1::Session {};

	if (!checkStatus(client->foundationKit->Login(&ctx2, req2, &repl2))) {
		return;
	}

	client->userToken = repl2.session_token();
	client->userID = repl2.user_id();

	QtConcurrent::run([=]() {
		client->runEvents();
	});
}

Client* Client::mainClient;
QMap<QString,Client*> Client::clients;

bool Client::createGuild(const QString &name)
{
	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::core::v1::CreateGuildRequest{};
	req.set_guild_name(name.toStdString());

	auto resp = protocol::core::v1::CreateGuildResponse{};

	if (!checkStatus(coreKit->CreateGuild(&ctx, req, &resp))) {
		return false;
	}

	ClientContext ctx2;
	authenticate(ctx2);

	auto req2 = protocol::core::v1::AddGuildToGuildListRequest{};
	req2.set_guild_id(resp.guild_id());
	req2.set_homeserver(homeserver.toStdString());

	auto resp2 = protocol::core::v1::AddGuildToGuildListResponse{};

	if (!checkStatus(coreKit->AddGuildToGuildList(&ctx2, req2, &resp2))) {
		return false;
	}

	return true;
}

bool Client::joinInvite(const QString& invite)
{
	protocol::core::v1::JoinGuildResponse resp;
	{
		ClientContext ctx;
		authenticate(ctx);

		protocol::core::v1::JoinGuildRequest req;
		req.set_invite_id(invite.toStdString());

		if (!checkStatus(coreKit->JoinGuild(&ctx, req, &resp))) {
			return false;
		}
	}
	{
		auto client = mainInstance();

		ClientContext ctx;
		client->authenticate(ctx);

		protocol::core::v1::AddGuildToGuildListRequest req;
		req.set_guild_id(resp.guild_id());
		req.set_homeserver(homeserver.toStdString());

		protocol::core::v1::AddGuildToGuildListResponse resp2;

		if (!checkStatus(client->coreKit->AddGuildToGuildList(&ctx, req, &resp2))) {
			return false;
		}
	}

	return true;
}

bool Client::leaveGuild(quint64 id, bool isOwner)
{
	if (!isOwner) {
		ClientContext ctx;
		authenticate(ctx);

		protocol::core::v1::LeaveGuildRequest req;
		req.set_guild_id(id);
		google::protobuf::Empty resp;

		if (!checkStatus(coreKit->LeaveGuild(&ctx, req, &resp))) {
			return false;
		}
	} else {
		ClientContext ctx;
		authenticate(ctx);

		protocol::core::v1::DeleteGuildRequest req;
		req.set_guild_id(id);
		google::protobuf::Empty resp;

		if (!checkStatus(coreKit->DeleteGuild(&ctx, req, &resp))) {
			return false;
		}
	}

	auto client = mainInstance();

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::core::v1::RemoveGuildFromGuildListRequest req;
	req.set_guild_id(id);
	req.set_homeserver(homeserver.toStdString());
	protocol::core::v1::RemoveGuildFromGuildListResponse resp;

	if (!checkStatus(client->coreKit->RemoveGuildFromGuildList(&ctx, req, &resp))) {
		return false;
	}

	return true;
}

bool Client::consumeToken(const QString& token, quint64 userID, const QString& homeserver)
{
	client = grpc::CreateChannel(homeserver.toStdString(), grpc::SslCredentials(grpc::SslCredentialsOptions()));
	clients[homeserver] = this;

	this->homeserver = homeserver;
	this->userID = userID;
	this->userToken = token.toStdString();

	coreKit = protocol::core::v1::CoreService::NewStub(client);
	foundationKit = protocol::foundation::v1::FoundationService::NewStub(client);
	profileKit = protocol::profile::v1::ProfileService::NewStub(client);

	if (!refreshGuilds()) {
		return false;
	}

	QtConcurrent::run([=]() {
		runEvents();
	});

	return true;
}

bool Client::hasPermission(const QString& node, quint64 guildID, quint64 channelID)
{
	ClientContext ctx;
	authenticate(ctx);

	protocol::core::v1::QueryPermissionsResponse resp;
	protocol::core::v1::QueryPermissionsRequest req;
	req.set_guild_id(guildID);
	if (channelID != 0) {
		req.set_channel_id(channelID);
	}
	req.set_check_for(node.toStdString());

	if (!checkStatus(coreKit->QueryHasPermission(&ctx, req, &resp))) {
		return false;
	}

	return resp.ok();
}

void Client::runEvents()
{
	ClientContext ctx;
	authenticate(ctx);

	eventStream = coreKit->StreamEvents(&ctx);
	protocol::core::v1::Event msg;

	protocol::core::v1::StreamEventsRequest req;
	req.set_allocated_subscribe_to_homeserver_events(new protocol::core::v1::StreamEventsRequest_SubscribeToHomeserverEvents);
	eventStream->Write(req, grpc::WriteOptions().set_write_through());

	while (eventStream->Read(&msg)) {
		if (msg.has_guild_added_to_list()) {
			auto ev = msg.guild_added_to_list();

			auto data = Client::instanceForHomeserver(QString::fromStdString(ev.homeserver()))->guildInfo(ev.guild_id());

			Q_EMIT State::instance()->guildModel->addGuild(Guild {
				.guildID = ev.guild_id(),
				.ownerID = data.ownerID,
				.homeserver = QString::fromStdString(ev.homeserver()),
				.name = data.name,
				.picture = data.picture,
			});
		} else if (msg.has_guild_removed_from_list()) {
			auto ev = msg.guild_removed_from_list();

			Q_EMIT State::instance()->guildModel->removeGuild(QString::fromStdString(ev.homeserver()), ev.guild_id());
		} else if (msg.has_action_performed()) {
			// we don't care about these
		} else if (msg.has_sent_message()) {
			auto ev = msg.sent_message();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.message().guild_id()), new MessageSentEvent(ev));
		} else if (msg.has_edited_message()) {
			auto ev = msg.edited_message();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new MessageUpdatedEvent(ev));
		} else if (msg.has_deleted_message()) {
			auto ev = msg.deleted_message();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new MessageDeletedEvent(ev));
		} else if (msg.has_created_channel()) {
			auto ev = msg.created_channel();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new ChannelCreatedEvent(ev));
		} else if (msg.has_edited_channel()) {
			auto ev = msg.edited_channel();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new ChannelUpdatedEvent(ev));
		} else if (msg.has_deleted_channel()) {
			auto ev = msg.deleted_channel();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new ChannelDeletedEvent(ev));
		} else if (msg.has_edited_guild()) {
			auto ev = msg.edited_guild();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new GuildUpdatedEvent(ev));
		} else if (msg.has_deleted_guild()) {
			// auto ev = msg.deleted_guild();

			// Q_UNUSED(ev)
		} else if (msg.has_joined_member()) {
			auto ev = msg.joined_member();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new MemberJoinedEvent(ev));
		} else if (msg.has_left_member()) {
			auto ev = msg.left_member();

			QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new MemberLeftEvent(ev));
		}
	}

	qDebug() << "Got to end of stream!";
}

void Client::subscribeGuild(quint64 guild)
{
	protocol::core::v1::StreamEventsRequest req;
	auto subReq = new protocol::core::v1::StreamEventsRequest_SubscribeToGuild;
	subReq->set_guild_id(guild);
	req.set_allocated_subscribe_to_guild(subReq);

	eventStream->Write(req, grpc::WriteOptions().set_write_through());
}

bool Client::login(const QString &email, const QString &password, const QString &hs)
{
	client = grpc::CreateChannel(hs.toStdString(), grpc::SslCredentials(grpc::SslCredentialsOptions()));
	clients[hs] = this;

	homeserver = hs;

	coreKit = protocol::core::v1::CoreService::NewStub(client);
	foundationKit = protocol::foundation::v1::FoundationService::NewStub(client);
	profileKit = protocol::profile::v1::ProfileService::NewStub(client);

	auto req = protocol::foundation::v1::LoginRequest{};
	auto local = new protocol::foundation::v1::LoginRequest_Local{};
	local->set_email(email.toStdString());
	local->set_password(password.toStdString());
	req.set_allocated_local(local);

	ClientContext ctx;
	protocol::foundation::v1::Session repl;

	if (!checkStatus(foundationKit->Login(&ctx, req, &repl))) {
		return false;
	}

	userToken = repl.session_token();
	userID = repl.user_id();

	QSettings settings;
	settings.setValue("state/token", QString::fromStdString(repl.session_token()));
	settings.setValue("state/homeserver", hs);
	settings.setValue("state/userid", userID);

	QtConcurrent::run([=]() {
		runEvents();
	});

	refreshGuilds();

	return true;
}
