// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QtCore>
#include <QEvent>
#include <QJSValue>
#include <optional>
#include <variant>

#include "client.hpp"

#include "protos.hpp"

template<int id, class T>
class CarrierEvent : public QEvent {
public:
	static constexpr int typeID = QEvent::User + id;
	CarrierEvent(T in) : QEvent(QEvent::Type(QEvent::User + id)), data(in) {}
	T data;
};

using GuildAddedToListEvent = CarrierEvent<1,protocol::chat::v1::Event_GuildAddedToList>;
using GuildRemovedFromListEvent = CarrierEvent<2,protocol::chat::v1::Event_GuildRemovedFromList>;
using ActionPerformedEvent = CarrierEvent<3,protocol::chat::v1::Event_ActionPerformed>;
using MessageSentEvent = CarrierEvent<4,protocol::chat::v1::Event_MessageSent>;
using MessageUpdatedEvent = CarrierEvent<5,protocol::chat::v1::Event_MessageUpdated>;
using MessageDeletedEvent = CarrierEvent<6,protocol::chat::v1::Event_MessageDeleted>;
using ChannelCreatedEvent = CarrierEvent<7,protocol::chat::v1::Event_ChannelCreated>;
using ChannelUpdatedEvent = CarrierEvent<8,protocol::chat::v1::Event_ChannelUpdated>;
using ChannelDeletedEvent = CarrierEvent<9,protocol::chat::v1::Event_ChannelDeleted>;
using GuildUpdatedEvent = CarrierEvent<10,protocol::chat::v1::Event_GuildUpdated>;
using GuildDeletedEvent = CarrierEvent<11,protocol::chat::v1::Event_GuildDeleted>;
using MemberJoinedEvent = CarrierEvent<12,protocol::chat::v1::Event_MemberJoined>;
using MemberLeftEvent = CarrierEvent<13,protocol::chat::v1::Event_MemberLeft>;
using TypingEvent = CarrierEvent<50, protocol::chat::v1::Event_Typing>;

struct PleaseCall {
	QJSValue func;
	QVariantList args;
};
using PleaseCallEvent = CarrierEvent<14,PleaseCall>;

bool checkStatusImpl(const char* file, int line, grpc::Status status);
#define checkStatus(in) checkStatusImpl(__FILE__, __LINE__, in)

using ExecuteEvent = CarrierEvent<15,std::function<void()>>;
void runOnMainThread(std::function<void()>);

struct GuildUpdate {
	QString homeserver;
	quint64 guildID;

	std::optional<QString> picture;
	std::optional<QString> name;
};

using GuildListUpdateEvent = CarrierEvent<16,GuildUpdate>;

template <typename T>
T withGuildAndUserID(quint64 guildID, quint64 userID) {
	T t;
	t.set_guild_id(guildID);
	t.set_user_id(userID);
	return t;
}

template <typename T>
bool resultOk(const T& t) {
	return !std::holds_alternative<QString>(t);
}

#define unwrap(t) std::get<0>(t)
