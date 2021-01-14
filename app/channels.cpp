// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJSEngine>
#include <QtConcurrent>
#include "channels.hpp"

#include "roles.hpp"
#include "invites.hpp"
#include "client.hpp"
#include "util.hpp"
#include "state.hpp"

#define doContext ClientContext ctx; client->authenticate(ctx)

using grpc::ClientContext;

MembersModel::MembersModel(QString homeserver, quint64 guildID, ChannelsModel* model) : QAbstractListModel(), homeServer(homeserver), guildID(guildID), model(model)
{
	client = Client::instanceForHomeserver(homeServer);

	QtConcurrent::run([=] {
		ClientContext ctx;
		client->authenticate(ctx);

		protocol::chat::v1::GetGuildMembersRequest req;
		req.set_guild_id(guildID);
		protocol::chat::v1::GetGuildMembersResponse resp;

		checkStatus(client->chatKit->GetGuildMembers(&ctx, req, &resp));

		beginResetModel();
		for (auto member : resp.members()) {
			members << member;
		}
		endResetModel();
	});
}

int MembersModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return members.count();
}

void MembersModel::customEvent(QEvent *event)
{
	if (event->type() == MemberJoinedEvent::typeID) {
		auto ev = reinterpret_cast<MemberJoinedEvent*>(event);
		beginInsertRows(QModelIndex(), members.length(), members.length());
		members << ev->data.member_id();
		endInsertRows();
	} else if (event->type() == MemberLeftEvent::typeID) {
		auto ev = reinterpret_cast<MemberLeftEvent*>(event);
		auto idx = std::find_if(members.begin(), members.end(), [=](quint64 id) { return id == ev->data.member_id(); });

		beginRemoveRows(QModelIndex(), idx - members.begin(), idx - members.begin());
		members.removeAt(idx - members.begin());
		endRemoveRows();
	} else if (event->type() == GuildUpdatedEvent::typeID) {
		auto ev = reinterpret_cast<GuildUpdatedEvent*>(event);
		if (ev->data.update_name()) {
			_name = QString::fromStdString(ev->data.name());
			Q_EMIT nameChanged();
		}
		if (ev->data.update_picture()) {
			_picture = State::instance()->transformHMCURL(QString::fromStdString(ev->data.picture()), homeServer);
			Q_EMIT pictureChanged();
		}
	} else if (event->type() == ExecuteEvent::typeID) {
		auto ev = reinterpret_cast<ExecuteEvent*>(event);
		ev->data();
	}
}

QVariant MembersModel::data(const QModelIndex& index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case MemberNameRole:
		return model->userName(members[index.row()]);
	case MemberAvatarRole:
		return model->avatarURL(members[index.row()]);
	}

	return QVariant();
}

ChannelsModel::ChannelsModel(QString homeServer, quint64 guildID) : QAbstractListModel(), homeServer(homeServer), guildID(guildID)
{
	client = Client::instanceForHomeserver(homeServer);
	nam = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager);
	members = new MembersModel(homeServer, guildID, this);
	permissions = new QQmlPropertyMap(this);

	permissions->insert("canCreate", client->hasPermission("channels.manage.create", guildID));
	permissions->insert("canMove", client->hasPermission("channels.manage.move", guildID));
	permissions->insert("canViewInvites", client->hasPermission("invites.view", guildID));
	permissions->insert("canManageRoles", client->hasPermission("roles.manage", guildID));

	auto& guilds = State::instance()->getGuildModel()->guilds;
	for (auto guild : guilds) {
		auto tc = guild.homeserver;
		if (guild.homeserver.isEmpty()) {
			tc = State::instance()->client->homeserver;
		}
		if (tc == homeServer && guild.guildID == guildID) {
			members->_name = guild.name;
			members->_picture = State::instance()->transformHMCURL(guild.picture, homeServer);
		}
	}

	QtConcurrent::run([=] {
		ClientContext ctx;
		client->authenticate(ctx);

		protocol::chat::v1::GetGuildChannelsRequest req;
		req.set_guild_id(guildID);

		protocol::chat::v1::GetGuildChannelsResponse resp;
		checkStatus(client->chatKit->GetGuildChannels(&ctx, req, &resp));

		QCoreApplication::postEvent(this, new ExecuteEvent([resp, this]{
			beginResetModel();
			for (auto chan : resp.channels()) {
				channels << Channel {
					.channelID = chan.channel_id(),
					.name = QString::fromStdString(chan.channel_name()),
					.isCategory = chan.is_category(),
				};
			}
			endResetModel();
		}));
	});

	client->subscribeGuild(guildID);
	instances.insert(qMakePair(homeServer, guildID), this);
}

QMap<QPair<QString,quint64>,ChannelsModel*> ChannelsModel::instances;

void ChannelsModel::moveChannelFromTo(int from, int to)
{
	QtConcurrent::run([=] {
		auto fromChan = channels[from];
		doContext;
		protocol::chat::v1::UpdateChannelOrderRequest req;
		google::protobuf::Empty resp;
		req.set_guild_id(guildID);
		req.set_channel_id(fromChan.channelID);

		if (to == 0) {
			req.set_next_id(channels[0].channelID);
		} else if (to + 1 == channels.length()) {
			req.set_previous_id(channels[to].channelID);
		} else {
			req.set_previous_id(channels[to-1].channelID);
			req.set_next_id(channels[to+1].channelID);
		}

		checkStatus(client->chatKit->UpdateChannelOrder(&ctx, req, &resp));
	});
}

void ChannelsModel::grabInstantView(const QString& url, QJSValue then)
{
	QtConcurrent::run([url, this](QJSValue then) {
		doContext;

		protocol::mediaproxy::v1::InstantViewRequest req;
		protocol::mediaproxy::v1::InstantViewResponse resp;

		req.set_url(url.toStdString());
		if (!checkStatus(client->mediaProxyKit->InstantView(&ctx, req, &resp))) {
			return;
		}
		if (!resp.is_valid()) {
			return;
		}

		callJS(then, {QString::fromStdString(resp.content())});
	}, then);
}

void ChannelsModel::checkCanInstantView(const QStringList& url, QJSValue then)
{
	QtConcurrent::run([url, this](QJSValue then) {
		QVariantList list;
		for (auto item : url) {
			QJsonObject obj;
			bool canInstantView = false;
			obj["from_url"] = item;

			{
				doContext;

				protocol::mediaproxy::v1::InstantViewRequest req;
				protocol::mediaproxy::v1::CanInstantViewResponse resp;

				req.set_url(item.toStdString());

				if (checkStatus(client->mediaProxyKit->CanInstantView(&ctx, req, &resp))) {
					obj["instant_view_ok"] = resp.can_instant_view();
					canInstantView = resp.can_instant_view();
				} else {
					obj["instant_view_ok"] = false;
				}
			}
			{
				doContext;

				protocol::mediaproxy::v1::FetchLinkMetadataRequest req;
				protocol::mediaproxy::v1::SiteMetadata resp;

				req.set_url(item.toStdString());

				checkStatus(client->mediaProxyKit->FetchLinkMetadata(&ctx, req, &resp));

				obj["page_title"] = QString::fromStdString(resp.page_title());
				obj["site_title"] = QString::fromStdString(resp.site_title());
				obj["description"] = QString::fromStdString(resp.description());
				obj["preview_image"] = QString::fromStdString(resp.image());
				obj["url"] = QString::fromStdString(resp.url());
			}

			list << obj;
		}
		callJS(then, {list});
	}, then);
}

void ChannelsModel::customEvent(QEvent *event)
{
	if (event->type() == ChannelCreatedEvent::typeID) {
		auto ev = reinterpret_cast<ChannelCreatedEvent*>(event);
		auto idx = (std::find_if(channels.begin(), channels.end(), [=](Channel& chan) { return chan.channelID == ev->data.previous_id(); }) - channels.begin());
		idx++;
		beginInsertRows(QModelIndex(), idx, idx);
		channels.insert(idx, Channel{
			.channelID = ev->data.channel_id(),
			.name = QString::fromStdString(ev->data.name()),
			.isCategory = ev->data.is_category()
		});
		endInsertRows();
	} else if (event->type() == ChannelDeletedEvent::typeID) {
		auto ev = reinterpret_cast<ChannelDeletedEvent*>(event);
		auto idx = std::find_if(channels.begin(), channels.end(), [=](Channel &chan) { return chan.channelID == ev->data.channel_id(); });
		beginRemoveRows(QModelIndex(), idx - channels.begin(), idx - channels.begin());
		channels.removeAt(idx - channels.begin());
		endRemoveRows();
	} else if (event->type() == MessageSentEvent::typeID) {
		auto ev = reinterpret_cast<MessageSentEvent*>(event);

		auto chanID = ev->data.message().channel_id();
		if (models.contains(chanID)) {
			QCoreApplication::postEvent(models[chanID], new MessageSentEvent(ev->data));
		}
	} else if (event->type() == MessageUpdatedEvent::typeID) {
		auto ev = reinterpret_cast<MessageUpdatedEvent*>(event);

		auto chanID = ev->data.channel_id();
		if (models.contains(chanID)) {
			QCoreApplication::postEvent(models[chanID], new MessageUpdatedEvent(ev->data));
		}
	} else if (event->type() == MessageDeletedEvent::typeID) {
		auto ev = reinterpret_cast<MessageDeletedEvent*>(event);

		auto chanID = ev->data.channel_id();
		if (models.contains(chanID)) {
			QCoreApplication::postEvent(models[chanID], new MessageDeletedEvent(ev->data));
		}
	} else if (event->type() == MemberJoinedEvent::typeID) {
		auto ev = reinterpret_cast<MemberJoinedEvent*>(event);

		QCoreApplication::postEvent(members, new MemberJoinedEvent(ev->data));
	} else if (event->type() == MemberLeftEvent::typeID) {
		auto ev = reinterpret_cast<MemberLeftEvent*>(event);

		QCoreApplication::postEvent(members, new MemberLeftEvent(ev->data));
	} else if (event->type() == GuildUpdatedEvent::typeID) {
		auto ev = reinterpret_cast<GuildUpdatedEvent*>(event);
		std::optional<QString> name;
		std::optional<QString> picture;

		if (ev->data.update_picture()) {
			picture = QString::fromStdString(ev->data.picture());
		}
		if (ev->data.update_name()) {
			name = QString::fromStdString(ev->data.name());
		}

		GuildUpdate upd;
		upd.homeserver = homeServer;
		upd.guildID = guildID;
		upd.name = name;
		upd.picture = picture;

		QCoreApplication::postEvent(members, new GuildUpdatedEvent(ev->data));
		QCoreApplication::postEvent(State::instance()->getGuildModel(), new GuildListUpdateEvent(upd));
	} else if (event->type() == ExecuteEvent::typeID) {
		auto ev = reinterpret_cast<ExecuteEvent*>(event);
		ev->data();
	} else if (event->type() == TypingEvent::typeID) {
		auto ev = reinterpret_cast<TypingEvent*>(event);

		auto chanID = ev->data.channel_id();
		if (models.contains(chanID)) {
			QCoreApplication::postEvent(models[chanID], new TypingEvent(ev->data));
		}
	}
}

int ChannelsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	return channels.count();
}

void ChannelsModel::setGuildPicture(const QString& url)
{
	QtConcurrent::run([=] {
		protocol::chat::v1::UpdateGuildInformationRequest req;
		req.set_guild_id(guildID);
		req.set_new_guild_picture(url.toStdString());
		req.set_update_guild_picture(true);

		google::protobuf::Empty resp;

		doContext;
		checkStatus(client->chatKit->UpdateGuildInformation(&ctx, req, &resp));
	});
}

QVariant ChannelsModel::data(const QModelIndex &index, int role) const
{
	if (!checkIndex(index))
		return QVariant();

	switch (role)
	{
	case ChannelIDRole:
		return QString::number(channels[index.row()].channelID);
	case ChannelNameRole:
		return channels[index.row()].name;
	case ChannelIsCategoryRole:
		return channels[index.row()].isCategory;
	}

	return QVariant();
}

MessagesModel* ChannelsModel::messagesModel(quint64 id) {
	if (!models.contains(id)) {
		models[id] = new MessagesModel(const_cast<ChannelsModel*>(this), homeServer, guildID, id);
	}
	return models[id];
}

QHash<int, QByteArray> ChannelsModel::roleNames() const
{
	QHash<int,QByteArray> ret;
	ret[ChannelIDRole] = "channelID";
	ret[ChannelNameRole] = "channelName";
	ret[ChannelIsCategoryRole] = "isCategory";

	return ret;
}

void ChannelsModel::deleteChannel(const QString& channel)
{
	QtConcurrent::run([=] {
		doContext;

		protocol::chat::v1::DeleteChannelRequest req;
		req.set_guild_id(this->guildID);
		req.set_channel_id(channel.toULongLong());

		google::protobuf::Empty resp;
		checkStatus(client->chatKit->DeleteChannel(&ctx, req, &resp));
	});
}

void ChannelsModel::createChannel(const QString& name, QJSValue then, QJSValue elseDo)
{
	QtConcurrent::run([=] {
		doContext;

		quint64 last = 0;

		for (auto channel : channels) {
			last = channel.channelID;
			if (channel.isCategory) {
				break;
			}
		}

		protocol::chat::v1::CreateChannelRequest req;
		req.set_guild_id(this->guildID);
		req.set_channel_name(name.toStdString());
		req.set_previous_id(last);

		protocol::chat::v1::CreateChannelResponse resp;

		if (checkStatus(client->chatKit->CreateChannel(&ctx, req, &resp))) {
			callJS(then, {});
		} else {
			callJS(elseDo, {});
		}
	});
}

QString ChannelsModel::userName(quint64 id)
{
	if (!users.contains(id)) {
		doContext;

		protocol::chat::v1::GetUserRequest req;
		req.set_user_id(id);

		protocol::chat::v1::GetUserResponse resp;

		checkStatus(client->chatKit->GetUser(&ctx, req, &resp));

		users[id] = QString::fromStdString(resp.user_name());
		avatars[id] = QString::fromStdString(resp.user_avatar());
	}
	return users[id];
}

QString ChannelsModel::avatarURL(quint64 id)
{
	if (!users.contains(id)) {
		doContext;

		protocol::chat::v1::GetUserRequest req;
		req.set_user_id(id);

		protocol::chat::v1::GetUserResponse resp;

		checkStatus(client->chatKit->GetUser(&ctx, req, &resp));

		users[id] = QString::fromStdString(resp.user_name());
		avatars[id] = QString::fromStdString(resp.user_avatar());
	}
	return State::instance()->transformHMCURL(avatars[id], homeServer);
}

InviteModel* ChannelsModel::invitesModel()
{
	return new InviteModel(this, homeServer, guildID);
}

RolesModel* ChannelsModel::rolesModel()
{
	return new RolesModel(homeServer, guildID);
}

void ChannelsModel::uploadFile(const QUrl& url, QJSValue then, QJSValue elseDo, QJSValue progress, QJSValue finally)
{
	QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QFile* file(new QFile(url.toLocalFile()));
	file->open(QIODevice::ReadOnly);

	QHttpPart filePart;
	filePart.setBodyDevice(file);
	filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"file\""));
	filePart.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");

	mp->append(filePart);

	QUrlQuery query;
	query.addQueryItem("filename", url.fileName());
	query.addQueryItem("contentType", QMimeDatabase().mimeTypeForFile(url.toLocalFile()).name());

	QUrl reqUrl("https://" + homeServer + "/_harmony/media/upload?" + query.query());
	QNetworkRequest req(reqUrl);
	req.setRawHeader(QByteArrayLiteral("Authorization"), QString::fromStdString(client->userToken).toLocal8Bit());

	auto reply = nam->post(req, mp);

	connect(reply, &QNetworkReply::uploadProgress, this, [=](qint64 sent, qint64 total) mutable {
		progress.call(QList<QJSValue>{ QJSValue(double(sent) / double(total)) });
	});
	connect(reply, &QNetworkReply::finished, this, [=]() mutable {
		auto data = reply->readAll();

		delete mp;
		delete file;
		delete reply;

		if (reply->error() != QNetworkReply::NoError) {
			elseDo.call();
			finally.call();
			return;
		}

		then.call(QList<QJSValue>{ QString("hmc://%1/%2").arg(homeServer).arg(QJsonDocument::fromJson(data)["id"].toString()) });
		finally.call();
		return;
	});
}
