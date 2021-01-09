// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QtConcurrent>
#include <QDebug>

#include "qcoreapplication.h"
#include "qloggingcategory.h"
#include "state.hpp"
#include "client.hpp"
#include "util.hpp"
#include "channels.hpp"
#include <grpc/impl/codegen/connectivity_state.h>
#include <unistd.h>
#include "logging.hpp"

using grpc::ClientContext;

class LoopFinished : public QEvent {
public:
	LoopFinished() : QEvent(QEvent::MaxUser) {}
};

class StartLoop : public QEvent {
public:
	StartLoop() : QEvent(QEvent::MaxUser) {}
};

class Subscribe : public QEvent {
public:
	Subscribe(quint64 gid) : QEvent(QEvent::MaxUser), gid(gid) {}
	quint64 gid;
};

void Client::authenticate(grpc::ClientContext &ctx)
{
	ctx.AddMetadata("auth", userToken);
}

bool Client::refreshGuilds()
{
	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::chat::v1::GetGuildListRequest{};
	protocol::chat::v1::GetGuildListResponse resp;

	if (!checkStatus(chatKit->GetGuildList(&ctx, req, &resp))) {
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
	qCDebug(CLIENT_LIFECYCLE) << "Constructing new client" << this;
}

Client* Client::mainInstance()
{
	if (mainClient == nullptr) {
		mainClient = new Client;
	}

	return mainClient;
}

Client* Client::instanceForHomeserver(const QString& homeserver)
{
	if (homeserver == "local" || homeserver.isEmpty()) {
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

	auto req = protocol::chat::v1::GetGuildRequest {};
	req.set_guild_id(id);

	auto repl = protocol::chat::v1::GetGuildResponse {};

	if (!checkStatus(chatKit->GetGuild(&ctx, req, &repl))) {
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
	qCDebug(CLIENT_LIFECYCLE) << this << "Federating new client" << client << "to" << target << "from" << homeserver;

	client->client = grpc::CreateChannel(target.toStdString(), grpc::SslCredentials(grpc::SslCredentialsOptions()));
	client->homeserver = target;
	client->chatKit = protocol::chat::v1::ChatService::NewStub(client->client);
	client->authKit = protocol::auth::v1::AuthService::NewStub(client->client);
	client->mediaProxyKit = protocol::mediaproxy::v1::MediaProxyService::NewStub(client->client);

	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::auth::v1::FederateRequest{};
	req.set_target(target.toStdString());
	auto repl = protocol::auth::v1::FederateReply{};

	if (!checkStatus(authKit->Federate(&ctx, req, &repl))) {
		return;
	}

	ClientContext ctx2;

	auto req2 = protocol::auth::v1::LoginFederatedRequest {};
	auto repl2 = protocol::auth::v1::Session {};

	if (!checkStatus(client->authKit->LoginFederated(&ctx2, req2, &repl2))) {
		return;
	}

	client->userToken = repl2.session_token();
	client->userID = repl2.user_id();

	client->runEvents();
}

Client* Client::mainClient;
QMap<QString,Client*> Client::clients;

bool Client::createGuild(const QString &name)
{
	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::chat::v1::CreateGuildRequest{};
	req.set_guild_name(name.toStdString());

	auto resp = protocol::chat::v1::CreateGuildResponse{};

	if (!checkStatus(chatKit->CreateGuild(&ctx, req, &resp))) {
		return false;
	}

	ClientContext ctx2;
	authenticate(ctx2);

	auto req2 = protocol::chat::v1::AddGuildToGuildListRequest{};
	req2.set_guild_id(resp.guild_id());

	auto resp2 = protocol::chat::v1::AddGuildToGuildListResponse{};

	if (!checkStatus(chatKit->AddGuildToGuildList(&ctx2, req2, &resp2))) {
		return false;
	}

	return true;
}

bool Client::joinInvite(const QString& invite)
{
	protocol::chat::v1::JoinGuildResponse resp;
	{
		ClientContext ctx;
		authenticate(ctx);

		protocol::chat::v1::JoinGuildRequest req;
		req.set_invite_id(invite.toStdString());

		if (!checkStatus(chatKit->JoinGuild(&ctx, req, &resp))) {
			return false;
		}
	}
	{
		auto client = mainInstance();

		ClientContext ctx;
		client->authenticate(ctx);

		protocol::chat::v1::AddGuildToGuildListRequest req;
		req.set_guild_id(resp.guild_id());
		req.set_homeserver(homeserver.toStdString());

		protocol::chat::v1::AddGuildToGuildListResponse resp2;

		if (!checkStatus(client->chatKit->AddGuildToGuildList(&ctx, req, &resp2))) {
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

		protocol::chat::v1::LeaveGuildRequest req;
		req.set_guild_id(id);
		google::protobuf::Empty resp;

		if (!checkStatus(chatKit->LeaveGuild(&ctx, req, &resp))) {
			return false;
		}
	} else {
		ClientContext ctx;
		authenticate(ctx);

		protocol::chat::v1::DeleteGuildRequest req;
		req.set_guild_id(id);
		google::protobuf::Empty resp;

		if (!checkStatus(chatKit->DeleteGuild(&ctx, req, &resp))) {
			return false;
		}
	}

	auto client = mainInstance();

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::chat::v1::RemoveGuildFromGuildListRequest req;
	req.set_guild_id(id);
	req.set_homeserver(homeserver.toStdString());
	protocol::chat::v1::RemoveGuildFromGuildListResponse resp;

	if (!checkStatus(client->chatKit->RemoveGuildFromGuildList(&ctx, req, &resp))) {
		return false;
	}

	return true;
}

bool Client::forgeNewConnection()
{
	qCDebug(CLIENT_LIFECYCLE) << this << "Creating new gRPC channels for homeserver" << homeserver;

	client = grpc::CreateChannel(homeserver.toStdString(), grpc::SslCredentials(grpc::SslCredentialsOptions()));

	chatKit = protocol::chat::v1::ChatService::NewStub(client);
	authKit = protocol::auth::v1::AuthService::NewStub(client);
	mediaProxyKit = protocol::mediaproxy::v1::MediaProxyService::NewStub(client);

	return true;
}

bool Client::consumeToken(const QString& token, quint64 userID, const QString& homeserver)
{
	qCDebug(CLIENT_LIFECYCLE) << this << "Consuming a token for" << homeserver;

	clients[homeserver] = this;

	this->homeserver = homeserver;
	this->userID = userID;
	this->userToken = token.toStdString();

	forgeNewConnection();

	if (!refreshGuilds()) {
		return false;
	}

	runEvents();

	return true;
}

bool Client::hasPermission(const QString& node, quint64 guildID, quint64 channelID)
{
	ClientContext ctx;
	authenticate(ctx);

	protocol::chat::v1::QueryPermissionsResponse resp;
	protocol::chat::v1::QueryPermissionsRequest req;
	req.set_guild_id(guildID);
	if (channelID != 0) {
		req.set_channel_id(channelID);
	}
	req.set_check_for(node.toStdString());

	if (!checkStatus(chatKit->QueryHasPermission(&ctx, req, &resp))) {
		return false;
	}

	return resp.ok();
}

void Client::customEvent(QEvent *event)
{
	if (auto ev = dynamic_cast<LoopFinished*>(event)) {
		Q_UNUSED(ev);

		qCDebug(STREAM_LIFECYCLE) << "Loop finished for homeserver" << homeserver;
		loopRunning = false;
		if (shouldRunLoop) {
			qCDebug(STREAM_LIFECYCLE) << "Posting start loop request for homeserver" << homeserver;
			QCoreApplication::postEvent(this, new StartLoop());
			qCDebug(STREAM_LIFECYCLE) << "Reposting guild subscribe requests for homeserver" << homeserver;
			for (auto guild : subscribedGuilds) {
				qCDebug(STREAM_LIFECYCLE) << "Reposting subscribe" << guild << homeserver;
				QCoreApplication::postEvent(this, new Subscribe(guild));
			}
			subscribedGuilds.clear();
		}
	} else if (auto ev = dynamic_cast<StartLoop*>(event)) {
		Q_UNUSED(ev);

		qCDebug(STREAM_LIFECYCLE) << "Got a start loop request, forging new connections for homeserver" << homeserver;
		forgeNewConnection();
		runEvents();
	} else if (auto ev = dynamic_cast<Subscribe*>(event)) {
		if (subscribedGuilds.contains(ev->gid)) return;

		if (loopRunning) {
			protocol::chat::v1::StreamEventsRequest req;
			auto subReq = new protocol::chat::v1::StreamEventsRequest_SubscribeToGuild;
			subReq->set_guild_id(ev->gid);
			req.set_allocated_subscribe_to_guild(subReq);

			qCDebug(STREAM_LIFECYCLE) << "Subscribing to guild" << ev->gid << "on homeserver" << homeserver;

			writeMutex.lock();
			subscribedGuilds << ev->gid;
			eventStream->Write(req, grpc::WriteOptions().set_write_through());
			writeMutex.unlock();

			qCDebug(STREAM_LIFECYCLE) << "Now subscribed to guilds" << subscribedGuilds << "on homeserver" << homeserver;
		} else {
			qCDebug(STREAM_LIFECYCLE) << "Got a request to subscribe to guild on homeserver" << homeserver << "but the stream is closed";
			if (shouldRunLoop) {
				qCDebug(STREAM_LIFECYCLE) << "Posting a start loop request and another subscribe request...";
				QCoreApplication::postEvent(this, new StartLoop());
				QCoreApplication::postEvent(this, new Subscribe(ev->gid));
			}
		}
	}
}

void Client::runEvents()
{
	loopRunning = true;

	QtConcurrent::run([=] {
		ClientContext ctx;
		authenticate(ctx);

		qCDebug(STREAM_LIFECYCLE) << "Creating new stream for homeserver" << homeserver;
		eventStream = chatKit->StreamEvents(&ctx);

		protocol::chat::v1::Event msg;

		protocol::chat::v1::StreamEventsRequest req;
		req.set_allocated_subscribe_to_homeserver_events(new protocol::chat::v1::StreamEventsRequest_SubscribeToHomeserverEvents);

		qCDebug(STREAM_LIFECYCLE) << "Subscribing to homeserver events" << homeserver;
		writeMutex.lock();
		eventStream->Write(req, grpc::WriteOptions().set_write_through());
		writeMutex.unlock();

		qCDebug(STREAM_LIFECYCLE) << "Beginning loop for homeserver" << homeserver;
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

		qCDebug(STREAM_LIFECYCLE) << "Finishing loop, closing thread for homeserver" << homeserver;
		QCoreApplication::postEvent(this, new LoopFinished());
	});
}

void Client::stopEvents()
{
	shouldRunLoop = false;

	qCDebug(STREAM_LIFECYCLE) << "Requested shutting down events for homeserver" << homeserver;
	if (eventStream.get() != nullptr) {
		qCDebug(STREAM_LIFECYCLE) << "Finishing shutting down events for homeserver" << homeserver;
		eventStream->Finish();
		qCDebug(STREAM_LIFECYCLE) << "Shut down events for homeserver" << homeserver;
	}
}

void Client::subscribeGuild(quint64 guild)
{
	QCoreApplication::postEvent(this, new Subscribe(guild));
}

void Client::consumeSession(protocol::auth::v1::Session session, const QString& homeserver)
{
	qCDebug(CLIENT_LIFECYCLE) << this << "Consuming a saved session for" << homeserver;

	userToken = session.session_token();
	userID = session.user_id();

	QSettings settings;
	settings.setValue("state/token", QString::fromStdString(session.session_token()));
	settings.setValue("state/homeserver", homeserver);
	settings.setValue("state/userid", userID);

	runEvents();

	refreshGuilds();

	Q_EMIT State::instance()->loggedIn();
}

