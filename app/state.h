#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QQmlParserStatus>

#include "clientmanager.h"
#include "protos.h"

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

class State : public QObject
{

	Q_OBJECT

	Q_PROPERTY(GuildList* guildList READ guildList NOTIFY guildListChanged)
	Q_PROPERTY(GuildsStore* guildsStore READ guildsStore NOTIFY guildsStoreChanged)
	Q_PROPERTY(MembersStore* membersStore READ membersStore NOTIFY membersStoreChanged)

	struct Private;
	QSharedPointer<Private> d;

	void createComponents();
	void createModels();

public:

	State(QQmlEngine* parent = nullptr);
	~State();

	Q_INVOKABLE void doInitialLogin();
	Q_INVOKABLE void doLogin(const QString& homeserver);
	void handleStep(protocol::auth::v1::AuthStep step);

	Q_INVOKABLE Future<ChannelsModel*> channelsModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE Future<MembersModel*> membersModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE Future<MessagesModel*> messagesModelFor(QString host, QString guildID, QString channelID, QObject* it);
	Q_INVOKABLE Future<InviteModel*> inviteModelFor(QString host, QString guildID, QObject* it);
	Q_INVOKABLE Future<RolesModel*> rolesModelFor(QString host, QString guildID, QObject* it);

	Q_INVOKABLE QString mediaURL(const QString& id, const QString& homeserver);

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

};
