#pragma once

#include <QtCore>
#include <QEvent>
#include <optional>

#include "client.hpp"

#include "core.grpc.pb.h"
#include "core.pb.h"

template<int id, class T>
class CarrierEvent : public QEvent {
public:
	static constexpr int typeID = QEvent::User + id;
	CarrierEvent(T in) : QEvent(QEvent::Type(QEvent::User + id)), data(in) {}
	T data;
};

struct Location {
	std::optional<quint64> guildID = std::optional<quint64>();
	std::optional<quint64> channelID = std::optional<quint64>();
	std::optional<quint64> messageID = std::optional<quint64>();

	operator protocol::core::v1::Location*() const {
		auto loc = new protocol::core::v1::Location;
		if (guildID.has_value()) {
			loc->set_guild_id(guildID.value());
		}
		if (channelID.has_value()) {
			loc->set_channel_id(channelID.value());
		}
		if (messageID.has_value()) {
			loc->set_message_id(messageID.value());
		}
		return loc;
	}
};

bool checkStatus(grpc::Status status);
