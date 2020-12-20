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

	GuildModel* guildModel;
	Client* client;

	friend class GuildModel;
	friend class Client;

	static State* s_instance;

public:
	State();
	~State();

	static State* instance();

	Q_SIGNAL void loggedIn();

	Q_INVOKABLE bool startupLogin();
	Q_INVOKABLE bool login(const QString& email, const QString& password, const QString &homeserver);
	Q_INVOKABLE bool createGuild(const QString& name);
	Q_INVOKABLE bool joinGuild(const QString& inviteLink);
	Q_INVOKABLE bool leaveGuild(const QString& homeserver, const QString& id, bool isOwner);
	Q_INVOKABLE void bindTextDocument(QQuickTextDocument* doc);
	Q_INVOKABLE QString transformHMCURL(const QString& url, const QString& homeserver) {
		if (!url.startsWith("hmc://")) {
			return QString("http://%1/_harmony/media/download/%2").arg(homeserver).arg(url);
		}

		QString trimmed = url.mid(QString("hmc://").length());
		auto split = trimmed.split("/");
		if (split.length() != 2) {
			qWarning() << "Malformed HMC URL:" << url;
			return QString("");
		}
		return QString("http://%1/_harmony/media/download/%2").arg(split[0]).arg(split[1]);
	}
	Q_PROPERTY(GuildModel* guildModel READ getGuildModel CONSTANT)
	GuildModel* getGuildModel() const { return guildModel; }
};

void callJS(QJSValue func, QList<QVariant> args);
