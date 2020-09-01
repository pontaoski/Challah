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
};

class GuildModel : public QAbstractListModel
{
	Q_OBJECT

	QList<Guild> guilds;
	friend class Client;

public:
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

class Client : public QObject
{
	Q_OBJECT

	std::shared_ptr<grpc_impl::Channel> client;

	std::unique_ptr<protocol::core::v1::CoreService::Stub> coreKit;
	std::unique_ptr<protocol::foundation::v1::FoundationService::Stub> foundationKit;
	std::unique_ptr<protocol::profile::v1::ProfileService::Stub> profileKit;

public:
	bool login(const QString& email, const QString& password, const QString& homeserver);
	void refreshGuilds();
};