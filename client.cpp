#include <QtConcurrent>
#include <QDebug>

#include "state.hpp"
#include "client.hpp"
#include "util.hpp"

using grpc::ClientContext;

void Client::authenticate(grpc::ClientContext &ctx)
{
	ctx.AddMetadata("auth", userToken);
}

void Client::refreshGuilds()
{
	ClientContext ctx;
	authenticate(ctx);

	auto req = protocol::core::v1::GetGuildListRequest{};
	protocol::core::v1::GetGuildListResponse resp;

	if (!checkStatus(coreKit->GetGuildList(&ctx, req, &resp))) {
		return;
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
	auto loc = new protocol::core::v1::Location {};
	loc->set_guild_id(id);
	req.set_allocated_location(loc);

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
	client->client = grpc::CreateChannel(target.toStdString(), grpc::InsecureChannelCredentials());
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
		req.set_guild_id(resp.location().guild_id());
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
		req.set_allocated_location(Location {
			.guildID = id,
		});
		google::protobuf::Empty resp;

		if (!checkStatus(coreKit->LeaveGuild(&ctx, req, &resp))) {
			return false;
		}
	} else {
		ClientContext ctx;
		authenticate(ctx);

		protocol::core::v1::DeleteGuildRequest req;
		req.set_allocated_location(Location {
			.guildID = id,
		});
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

bool Client::login(const QString &email, const QString &password, const QString &hs)
{
	client = grpc::CreateChannel(hs.toStdString(), grpc::InsecureChannelCredentials());
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

	QtConcurrent::run([=]() {
		ClientContext ctx;
		authenticate(ctx);

		protocol::core::v1::StreamHomeserverEventsRequest req;
		auto stream = coreKit->StreamHomeserverEvents(&ctx, req);
		protocol::core::v1::HomeserverEvent msg;

		while (stream->Read(&msg)) {
			if (msg.has_guild_added_to_list()) {
				auto guild = msg.guild_added_to_list();

				auto data = Client::instanceForHomeserver(QString::fromStdString(guild.homeserver()))->guildInfo(guild.guild_id());

				Q_EMIT State::instance()->guildModel->addGuild(Guild {
					.guildID = guild.guild_id(),
					.ownerID = data.ownerID,
					.homeserver = QString::fromStdString(guild.homeserver()),
					.name = data.name,
					.picture = data.picture,
				});
			} else if (msg.has_guild_removed_from_list()) {
				auto guild = msg.guild_removed_from_list();

				Q_EMIT State::instance()->guildModel->removeGuild(QString::fromStdString(guild.homeserver()), guild.guild_id());
			}
		}
	});

	refreshGuilds();

	return true;
}