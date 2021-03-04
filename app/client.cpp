// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QtConcurrent>
#include <QDebug>

#include "messages.hpp"
#include "qcoreapplication.h"
#include "qloggingcategory.h"
#include "state.hpp"
#include "client.hpp"
#include "util.hpp"
#include "channels.hpp"
#include <unistd.h>
#include "logging.hpp"

#define theHeaders {{"Authorization", userToken}}

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

bool Client::refreshGuilds()
{
	auto req = protocol::chat::v1::GetGuildListRequest{};

	auto result = chatKit->GetGuildList(req, theHeaders);
	if (!resultOk(result)) {
		return false;
	}
	auto resp = unwrap(result);

	State::instance()->guildModel->guilds.clear();

	auto guilds = resp.guilds();
	for (auto guild : guilds)
	{
		auto data = Client::instanceForHomeserver(QString::fromStdString(guild.host()))->guildInfo(guild.guild_id());

		State::instance()->guildModel->guilds << Guild {
			guild.guild_id(),
			data.ownerID,
			QString::fromStdString(guild.host()),
			data.name,
			data.picture,
		};
	}

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
	auto req = protocol::chat::v1::GetGuildRequest {};
	req.set_guild_id(id);

	auto result = chatKit->GetGuild(req, theHeaders);
	if (!resultOk(result)) {
		return GuildRepl{};
	}
	auto resp = unwrap(result);

	return GuildRepl {
		resp.guild_owner(),
		QString::fromStdString(resp.guild_name()),
		QString::fromStdString(resp.guild_picture()),
	};
}

void Client::federateOtherClient(Client* client, const QString& target)
{
	qCDebug(CLIENT_LIFECYCLE) << this << "Federating new client" << client << "to" << target << "from" << homeserver;

	client->homeserver = target;
	client->chatKit = std::unique_ptr<ChatServiceServiceClient>(new ChatServiceServiceClient(target, true));
	client->authKit = std::unique_ptr<AuthServiceServiceClient>(new AuthServiceServiceClient(target, true));
	client->mediaProxyKit = std::unique_ptr<MediaProxyServiceServiceClient>(new MediaProxyServiceServiceClient(target, true));

	auto req = protocol::auth::v1::FederateRequest{};
	req.set_target(target.toStdString());

	auto result = authKit->Federate(req, theHeaders);
	if (!resultOk(result)) {
		return;
	}

	auto req2 = protocol::auth::v1::LoginFederatedRequest {};
	req2.set_auth_token(unwrap(result).token());
	req2.set_domain(homeserver.toStdString());

	auto result2 = client->authKit->LoginFederated(req2);
	if (!resultOk(result2)) {
		return;
	}
	auto resp = unwrap(result2);

	client->userToken = QString::fromStdString(resp.session_token());
	client->userID = resp.user_id();

	client->runEvents();
}

Client* Client::mainClient;
QMap<QString,Client*> Client::clients;

bool Client::createGuild(const QString &name)
{
	auto req = protocol::chat::v1::CreateGuildRequest{};
	req.set_guild_name(name.toStdString());

	auto result = chatKit->CreateGuild(req, theHeaders);
	if (!resultOk(result)) {
		return false;
	}
	auto resp = unwrap(result);

	auto req2 = protocol::chat::v1::AddGuildToGuildListRequest{};
	req2.set_guild_id(resp.guild_id());

	return resultOk(chatKit->AddGuildToGuildList(req2, theHeaders));
}

bool Client::joinInvite(const QString& invite)
{
	protocol::chat::v1::JoinGuildRequest req;
	req.set_invite_id(invite.toStdString());

	auto result = chatKit->JoinGuild(req, theHeaders);
	if (!resultOk(result)) {
		return false;
	}
	auto resp = unwrap(result);

	auto client = mainInstance();

	protocol::chat::v1::AddGuildToGuildListRequest req2;
	req2.set_guild_id(resp.guild_id());
	req2.set_homeserver(homeserver.toStdString());

	return resultOk(client->chatKit->AddGuildToGuildList(req2, {{"Authorization", client->userToken}}));

	return true;
}

bool Client::leaveGuild(quint64 id, bool isOwner)
{
	if (!isOwner) {
		protocol::chat::v1::LeaveGuildRequest req;
		req.set_guild_id(id);

		if (!resultOk(chatKit->LeaveGuild(req, theHeaders))) {
			return false;
		}
	} else {
		protocol::chat::v1::DeleteGuildRequest req;
		req.set_guild_id(id);

		if (!resultOk(chatKit->DeleteGuild(req, theHeaders))) {
			return false;
		}
	}

	auto client = mainInstance();

	protocol::chat::v1::RemoveGuildFromGuildListRequest req;
	req.set_guild_id(id);
	req.set_homeserver(homeserver.toStdString());

	return resultOk(client->chatKit->RemoveGuildFromGuildList(req, {{"Authorization", client->userToken}}));
}

bool Client::forgeNewConnection()
{
	qCDebug(CLIENT_LIFECYCLE) << this << "Creating new hRPC channels for homeserver" << homeserver;

	chatKit = std::unique_ptr<ChatServiceServiceClient>(new ChatServiceServiceClient(homeserver, !qEnvironmentVariableIsSet("CHALLAH_INSECURE")));
	authKit = std::unique_ptr<AuthServiceServiceClient>(new AuthServiceServiceClient(homeserver, !qEnvironmentVariableIsSet("CHALLAH_INSECURE")));
	mediaProxyKit = std::unique_ptr<MediaProxyServiceServiceClient>(new MediaProxyServiceServiceClient(homeserver, !qEnvironmentVariableIsSet("CHALLAH_INSECURE")));

	return true;
}

bool Client::consumeToken(const QString& token, quint64 userID, const QString& homeserver)
{
	qCDebug(CLIENT_LIFECYCLE) << this << "Consuming a token for" << homeserver;

	clients[homeserver] = this;

	this->homeserver = homeserver;
	this->userID = userID;
	this->userToken = token;

	forgeNewConnection();

	if (!refreshGuilds()) {
		return false;
	}

	runOnMainThread("running events...", [=] {
		runEvents();
	});

	return true;
}

bool Client::hasPermission(const QString& node, quint64 guildID, quint64 channelID)
{
	protocol::chat::v1::QueryPermissionsRequest req;
	req.set_guild_id(guildID);
	if (channelID != 0) {
		req.set_channel_id(channelID);
	}
	req.set_check_for(node.toStdString());

	auto result = chatKit->QueryHasPermission(req, theHeaders);
	if (!resultOk(result)) {
		return false;
	}

	return unwrap(result).ok();
}

void Client::customEvent(QEvent *event)
{
	if (auto ev = dynamic_cast<LoopFinished*>(event)) {
		Q_UNUSED(ev);

		qCDebug(STREAM_LIFECYCLE) << "Posting start loop request for homeserver" << homeserver;

		pendingSubscribeGuilds = subscribedGuilds;
		subscribedGuilds.clear();

		QCoreApplication::postEvent(this, new StartLoop());

	} else if (auto ev = dynamic_cast<StartLoop*>(event)) {
		Q_UNUSED(ev);

		runEvents();
	} else if (auto ev = dynamic_cast<Subscribe*>(event)) {
		if (subscribedGuilds.contains(ev->gid)) return;

		if (eventStream && eventStream->isValid() && eventStream->state() == QAbstractSocket::SocketState::ConnectedState) {
		before:
			qCDebug(STREAM_LIFECYCLE) << "Subscribing to guild" << ev->gid << "on homeserver" << homeserver;
			protocol::chat::v1::StreamEventsRequest req;
			auto subReq = new protocol::chat::v1::StreamEventsRequest_SubscribeToGuild;
			subReq->set_guild_id(ev->gid);
			req.set_allocated_subscribe_to_guild(subReq);

			if (!eventStream->send(req)) {
				qCDebug(STREAM_LIFECYCLE) << "Failed to subscribe to guild; reposting subscribe" << eventStream->state() << eventStream->errorString() << ev->gid << homeserver;
				QCoreApplication::postEvent(this, new Subscribe(ev->gid));
				goto after;
			}

			subscribedGuilds << ev->gid;
			qCDebug(STREAM_LIFECYCLE) << "Now subscribed to guilds" << subscribedGuilds << "on homeserver" << homeserver;

			goto after;
		} else {
			qCDebug(STREAM_LIFECYCLE) << "Got a request to subscribe to guild on homeserver" << homeserver << "but the stream is closed";
			qCDebug(STREAM_LIFECYCLE) << "Starting the stream...";

			runEvents();
			goto before;
		}
	after:
		;
	}
}

void Client::runEvents()
{
	qCDebug(STREAM_LIFECYCLE) << "Creating new stream for homeserver" << homeserver;
	eventStream = std::unique_ptr<Receive__protocol_chat_v1_Event__Send__protocol_chat_v1_StreamEventsRequest__Stream>(chatKit->StreamEvents(theHeaders));

	connect(
		eventStream.get(),
		&QWebSocket::connected,
		[=] {
			protocol::chat::v1::StreamEventsRequest req;
			req.set_allocated_subscribe_to_homeserver_events(new protocol::chat::v1::StreamEventsRequest_SubscribeToHomeserverEvents);

			qCDebug(STREAM_LIFECYCLE) << "Events stream opened; subscribing to homeserver events" << homeserver << eventStream->send(req);

			for (auto item : pendingSubscribeGuilds) {
				protocol::chat::v1::StreamEventsRequest req;
				auto subReq = new protocol::chat::v1::StreamEventsRequest_SubscribeToGuild;
				subReq->set_guild_id(item);
				req.set_allocated_subscribe_to_guild(subReq);

				if (!eventStream->send(req)) {
					qCDebug(STREAM_LIFECYCLE) << "Failed to resubscribe to guild; reposting subscribe" << eventStream->state() << eventStream->errorString() << item << homeserver;
					QCoreApplication::postEvent(this, new Subscribe(item));
				}

				subscribedGuilds << item;
			}
			pendingSubscribeGuilds.clear();
		}
	);

	connect(
		eventStream.get(),
		&Receive__protocol_chat_v1_Event__Send__protocol_chat_v1_StreamEventsRequest__Stream::receivedMessage,
		[=](const protocol::chat::v1::Event& msg) {
			if (msg.has_guild_added_to_list()) {
				auto ev = msg.guild_added_to_list();

				auto data = Client::instanceForHomeserver(QString::fromStdString(ev.homeserver()))->guildInfo(ev.guild_id());

				Q_EMIT State::instance()->guildModel->addGuild(Guild {
					ev.guild_id(),
					data.ownerID,
					QString::fromStdString(ev.homeserver()),
					data.name,
					data.picture,
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
			} else if (msg.has_typing()) {
				auto ev = msg.typing();

				QCoreApplication::postEvent(ChannelsModel::modelFor(homeserver, ev.guild_id()), new TypingEvent(ev));
			}
		}
	);
	connect(
		eventStream.get(),
		&QWebSocket::disconnected,
		[=] {
			qCDebug(STREAM_LIFECYCLE) << "Stream finished for homeserver" << homeserver;
			if (shouldRestartStreams) {
				qCDebug(STREAM_LIFECYCLE) << "Reconnecting in 500ms";
				QTimer::singleShot(500, [=] {
					qCDebug(STREAM_LIFECYCLE) << "Posting loop finished event" << homeserver;
					QCoreApplication::postEvent(this, new LoopFinished());
				});
			}
		}
	);
	connect(
		eventStream.get(),
		QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
		[=](QAbstractSocket::SocketError error) {
			if (error != QAbstractSocket::SocketError::ConnectionRefusedError) {
				qCDebug(STREAM_LIFECYCLE) << "Unknown error:" << error;
				return;
			}
			if (shouldRestartStreams) {
				qCDebug(STREAM_LIFECYCLE) << "Stream errored for homeserver, reconnecting in 500ms" << homeserver;
				QTimer::singleShot(500, [=] {
					qCDebug(STREAM_LIFECYCLE) << "Posting loop finished event" << homeserver;
					QCoreApplication::postEvent(this, new LoopFinished());
				});
			} else {
				qCDebug(STREAM_LIFECYCLE) << "Stream errored for homeserver, not reconnecting" << homeserver;
			}
		}
	);
	connect(
		eventStream.get(),
		&QWebSocket::stateChanged,
		[=](QAbstractSocket::SocketState state) {
			qCDebug(STREAM_LIFECYCLE) << "State changed for homeserver" << homeserver << ":" << state;
		}
	);
}

void Client::stopEvents()
{
	shouldRestartStreams = false;

	qCDebug(STREAM_LIFECYCLE) << "Requested shutting down events for homeserver" << homeserver;
	if (eventStream.get() != nullptr) {
		qCDebug(STREAM_LIFECYCLE) << "Finishing shutting down events for homeserver" << homeserver;
		eventStream->close();
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

	userToken = QString::fromStdString(session.session_token());
	userID = session.user_id();

	QSettings settings;
	settings.setValue("state/token", QString::fromStdString(session.session_token()));
	settings.setValue("state/homeserver", homeserver);
	settings.setValue("state/userid", userID);

	runEvents();

	refreshGuilds();

	Q_EMIT State::instance()->loggedIn();
}

