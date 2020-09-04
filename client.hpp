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

class Client;

struct Guild {
	quint64 guildID;
	QString homeserver;
	QString name;
	QString picture;
};

Q_DECLARE_METATYPE(Guild)

struct GuildRepl {
	QString name;
	QString picture;
};

class GuildModel : public QAbstractListModel
{
	Q_OBJECT

	QList<Guild> guilds;
	friend class Client;

	Q_SIGNAL void addGuild(Guild data);
	Q_SLOT void addGuildHandler(Guild data);

	enum Roles {
		GuildIDRole = Qt::UserRole,
		GuildNameRole,
		GuildPictureRole,
	};

public:
	GuildModel();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QHash<int,QByteArray> roleNames() const override;
};

class Client : public QObject
{
	Q_OBJECT

	static Client* mainClient;
	static QMap<QString,Client*> clients;

	quint64 userID;
	std::string userToken;

	std::shared_ptr<grpc_impl::Channel> client;
	QString homeserver;

	std::unique_ptr<protocol::core::v1::CoreService::Stub> coreKit;
	std::unique_ptr<protocol::foundation::v1::FoundationService::Stub> foundationKit;
	std::unique_ptr<protocol::profile::v1::ProfileService::Stub> profileKit;

	void authenticate(grpc::ClientContext& ctx);
	void federateOtherClient(Client* client, const QString& target);

	Client();

public:
	static Client* mainInstance();
	static Client* instanceForHomeserver(const QString& homeserver);
	bool login(const QString& email, const QString& password, const QString& homeserver);
	bool createGuild(const QString& name);
	GuildRepl guildInfo(quint64 id);
	void refreshGuilds();
};