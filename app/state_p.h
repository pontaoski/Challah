#pragma once

#include <QQmlComponent>

#include "state.h"
#include "clientmanager.h"
#include "guilds.h"
#include "channels.h"
#include "members_model.h"
#include "members_store.h"
#include "messages_model.h"
#include "roles.h"
#include "invites.h"
#include "own_permissions.h"

struct State::Private
{
	QSharedPointer<SDK::ClientManager> sdk;
	QMap<QPair<QString,quint64>,QPointer<ChannelsModel>> channelsModels;
	QMap<QPair<QString,quint64>,QPointer<MembersModel>> membersModels;
	QMap<QPair<QString,QPair<quint64,quint64>>,QPointer<MessagesModel>> messagesModels;
	GuildList* list = nullptr;
	GuildsStore* store = nullptr;
	MembersStore* membersStore = nullptr;
	OwnPermissionsStore* ownPermissionsStore = nullptr;
	OverridesModel* overridesModel = nullptr;
	QString homeserver;
	QString userID;

	QQmlEngine* eng = nullptr;
	QQmlComponent* kirigamiHeadingComponent = nullptr;
	QQmlComponent* columnLayoutComponent = nullptr;
	QQmlComponent* buttonComponent = nullptr;
	QQmlComponent* textFieldComponent = nullptr;
	QQmlComponent* kirigamiFormLayoutComponent = nullptr;
	QQmlComponent* passwordFieldComponent = nullptr;

	Private() {
		sdk = QSharedPointer<SDK::ClientManager>(new SDK::ClientManager, &QObject::deleteLater);
	}
};

class ConnHelper : public QObject {
	Q_OBJECT

	std::function<void()> func;

public:
	Q_SLOT void trigger() { func(); }
	ConnHelper(std::function<void()>&& fun, QObject* parent = nullptr) : QObject(parent), func(std::move(fun))
	{

	}

	static QMetaObject::Connection connect(
		QObject* sender,
		const char* signal,
		std::function<void()>&& func
	) {
		if (!sender) return QMetaObject::Connection();

		return QObject::connect(sender, signal, new ConnHelper(std::move(func), sender), SLOT(trigger()));
	}
};
