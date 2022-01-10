#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QQmlParserStatus>

#include "clientmanager.h"
#include "protos.h"
#include "coroutine_integration.h"
#include "coroutine_integration_network.h"

class QQuickItem;
class QQmlEngine;
class GuildList;
class GuildsStore;
class ChannelsModel;
class MembersModel;
class MembersStore;
class MessagesModel;
class InviteModel;
class RolesModel;
class OwnPermissionsStore;
class voiceCall;
class OverridesModel;

class State : public QObject
{

	Q_OBJECT

	Q_PROPERTY(OverridesModel* overridesModel READ overridesModel NOTIFY overridesModelChanged)
	Q_PROPERTY(GuildList* guildList READ guildList NOTIFY guildListChanged)
	Q_PROPERTY(GuildsStore* guildsStore READ guildsStore NOTIFY guildsStoreChanged)
	Q_PROPERTY(MembersStore* membersStore READ membersStore NOTIFY membersStoreChanged)
	Q_PROPERTY(OwnPermissionsStore* ownPermissionsStore READ ownPermissionsStore NOTIFY ownPermissionsStoreChanged)
	Q_PROPERTY(QString ownID READ ownID NOTIFY ownIDChanged)
	Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)

	struct Private;
	QSharedPointer<Private> d;

	void createComponents();
	void createModels();

public:

	State(QQmlEngine* parent = nullptr);
	~State();

	Q_INVOKABLE void doInitialLogin();
	Q_INVOKABLE void doLogin(const QString& homeserver);
	Q_INVOKABLE void logOut();
	void handleStep(protocol::auth::v1::AuthStep step);

	enum ConnectionStatus {
		Offline,
		PartialOutage,
		Connecting,
		Connected,
	};
	Q_ENUM(ConnectionStatus)

	ConnectionStatus connectionStatus() const;
	Q_SIGNAL void connectionStatusChanged();

	Q_INVOKABLE ChannelsModel* channelsModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE MembersModel* membersModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE MessagesModel* messagesModelFor(QString host, QString guildID, QString channelID, QObject* it);
	Q_INVOKABLE InviteModel* inviteModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE RolesModel* rolesModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE voiceCall* makeVoiceCall(QString host, QString guildID, QString channelID, QObject* it);

	OverridesModel* overridesModel();
	Q_SIGNAL void overridesModelChanged();

	Q_INVOKABLE QString mediaURL(const QString& id, const QString& homeserver);

	Q_INVOKABLE Croutons::FutureBase createGuild(QString name);
	Q_INVOKABLE Croutons::FutureBase joinGuild(QString name);

	Q_SIGNAL void beginHomeserver();
	Q_SIGNAL void beginLogin();
	Q_SIGNAL void placeItem(QQuickItem* item);
	Q_SIGNAL void endLogin();

	SDK::ClientManager* api();

	GuildList* guildList();
	Q_SIGNAL void guildListChanged();

	GuildsStore* guildsStore();
	Q_SIGNAL void guildsStoreChanged();

	MembersStore* membersStore();
	Q_SIGNAL void membersStoreChanged();

	QString ownID();
	Q_SIGNAL void ownIDChanged();

	OwnPermissionsStore* ownPermissionsStore();
	Q_SIGNAL void ownPermissionsStoreChanged();

};
