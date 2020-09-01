#include <QDebug>

#include "state.hpp"
#include "client.hpp"

using grpc::ClientContext;

int GuildModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)


}

QVariant GuildModel::data(const QModelIndex& index, int role) const
{

}

void Client::refreshGuilds()
{
    ClientContext ctx;
    auto req = protocol::core::v1::GetGuildListRequest {};
    protocol::core::v1::GetGuildListResponse resp;

    auto status = coreKit->GetGuildList(&ctx, req, &resp);
    if (!status.ok()) {
        qDebug() << status.error_code();
        qDebug() << status.error_details().c_str();
        qDebug() << status.error_message().c_str();
        return;
    }

    State::instance()->guildModel->guilds.clear();

    auto guilds = resp.guilds();
    for (auto guild : guilds) {
        State::instance()->guildModel->guilds << Guild {
            .guildID = guild.guild_id(),
            .homeserver = QString::fromStdString(guild.host()),
        };

        qDebug() << guild.guild_id() << guild.host().c_str();
    }
}

bool Client::login(const QString& email, const QString& password, const QString& homeserver)
{
    client = grpc::CreateChannel(homeserver.toStdString(), grpc::InsecureChannelCredentials());

    coreKit = protocol::core::v1::CoreService::NewStub(client);
    foundationKit = protocol::foundation::v1::FoundationService::NewStub(client);
    profileKit = protocol::profile::v1::ProfileService::NewStub(client);

    auto req = protocol::foundation::v1::LoginRequest {};
    auto local = new protocol::foundation::v1::LoginRequest_Local {};
    local->set_email(email.toStdString());
    local->set_password(password.toStdString());
    req.set_allocated_local(local);

    ClientContext ctx;
    protocol::foundation::v1::Session repl;

    auto status = foundationKit->Login(&ctx, req, &repl);
    if (!status.ok()) {
        qDebug() << status.error_code();
        qDebug() << status.error_details().c_str();
        qDebug() << status.error_message().c_str();

        return false;
    }

    refreshGuilds();

    return true;
}