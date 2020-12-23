// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QtCore>
#include <QEvent>
#include <QJSValue>
#include <optional>

#include "client.hpp"

#include "chat/v1/chat.grpc.pb.h"
#include "chat/v1/chat.pb.h"

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

struct PleaseCall {
	QJSValue func;
	QVariantList args;
};
using PleaseCallEvent = CarrierEvent<14,PleaseCall>;

bool checkStatus(grpc::Status status);
