// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QDebug>
#include <QObject>
#include "client.hpp"

class QQuickTextDocument;

class State : public QObject
{
	Q_OBJECT

	friend class GuildModel;
	friend class Client;
	friend class LoginManager;

	static State* s_instance;

protected:
	void customEvent(QEvent *event) override;

public:
	GuildModel* guildModel;
	Client* client;

	State();
	~State();

	static State* instance();

	Q_SIGNAL void loggedIn();
	Q_SIGNAL void loginFailure();
	Q_SIGNAL void loggedOut();

	Q_INVOKABLE void logOut();
	Q_INVOKABLE bool startupLogin();
	Q_INVOKABLE bool createGuild(const QString& name);
	Q_INVOKABLE bool joinGuild(const QString& inviteLink);
	Q_INVOKABLE bool leaveGuild(const QString& homeserver, const QString& id, bool isOwner);
	Q_INVOKABLE void bindTextDocument(QQuickTextDocument* doc, QObject* field);
	Q_INVOKABLE QString transformHMCURL(const QString& url, const QString& homeserver) {
		auto hs = homeserver;
		if (hs.isEmpty()) {
			hs = client->homeserver;
		}

		if (!url.startsWith("hmc://")) {
			return QString("https://%1/_harmony/media/download/%2").arg(hs).arg(url);
		}

		QString trimmed = url.mid(QString("hmc://").length());
		auto split = trimmed.split("/");
		if (split.length() != 2) {
			qWarning() << "Malformed HMC URL:" << url;
			return QString("");
		}
		return QString("https://%1/_harmony/media/download/%2").arg(split[0]).arg(split[1]);
	}
	Q_SIGNAL void guildModelChanged();
	Q_PROPERTY(GuildModel* guildModel READ getGuildModel NOTIFY guildModelChanged)
	GuildModel* getGuildModel() const { return guildModel; }
};

void callJS(QJSValue func, QList<QVariant> args);
