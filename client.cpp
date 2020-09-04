#include <QtConcurrent>
#include <QDebug>

#include "state.hpp"
#include "client.hpp"

using grpc::ClientContext;

GuildModel::GuildModel() : QAbstractListModel()
{
	static bool initted = false;
	if (!initted) {
		initted = true;
		qRegisterMetaType<Guild>();
	}
	connect(this, &GuildModel::addGuild, this, &GuildModel::addGuildHandler, Qt::QueuedConnection);
}

void GuildModel::addGuildHandler(Guild guild)
{
	beginInsertRows(
		QModelIndex(),
		guilds.count(),
		guilds.count()
	);
	guilds << guild;
	endInsertRows();
}

int GuildModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return guilds.count();
}

QVariant GuildModel::data(const QModelIndex &index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case GuildIDRole:
		return guilds[index.row()].guildID;
	case GuildNameRole:
		return guilds[index.row()].name;
	}

	return QVariant();
}

QHash<int, QByteArray> GuildModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[GuildIDRole] = "guildID";
	ret[GuildNameRole] = "guildName";

	return ret;
}

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

	auto status = coreKit->GetGuildList(&ctx, req, &resp);
	if (!status.ok())
	{
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();
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

	auto status = coreKit->GetGuild(&ctx, req, &repl);
	if (!status.ok()) {
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();

		return GuildRepl{};
	}

	return GuildRepl {
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

	auto status = foundationKit->Federate(&ctx, req, &repl);
	if (!status.ok()) {
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();

		return;
	}

	ClientContext ctx2;

	auto req2 = protocol::foundation::v1::LoginRequest {};
	auto remote = new protocol::foundation::v1::LoginRequest_Federated {};
	remote->set_auth_token(repl.token());
	remote->set_domain(homeserver.toStdString());
	req2.set_allocated_federated(remote);

	auto repl2 = protocol::foundation::v1::Session {};

	status = client->foundationKit->Login(&ctx2, req2, &repl2);

	if (!status.ok()) {
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();

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

	auto status = coreKit->CreateGuild(&ctx, req, &resp);
	if (!status.ok()) {
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();

		return false;
	}

	ClientContext ctx2;
	authenticate(ctx2);

	auto req2 = protocol::core::v1::AddGuildToGuildListRequest{};
	req2.set_guild_id(resp.guild_id());
	req2.set_homeserver(homeserver.toStdString());

	auto resp2 = protocol::core::v1::AddGuildToGuildListResponse{};
	
	status = coreKit->AddGuildToGuildList(&ctx2, req2, &resp2);
	if (!status.ok()) {
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();

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

	auto status = foundationKit->Login(&ctx, req, &repl);
	if (!status.ok())
	{
		qDebug() << status.error_code();
		qDebug() << status.error_details().c_str();
		qDebug() << status.error_message().c_str();

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
					.homeserver = QString::fromStdString(guild.homeserver()),
					.name = data.name,
					.picture = data.picture,
				});
			}
		}
	});

	refreshGuilds();

	return true;
}