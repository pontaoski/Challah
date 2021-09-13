#pragma once

#include <QMap>

#include "harmonytypes/v1/types.pb.h"
#include "messages.h"

struct MessagesStore::Private
{
	QMap<quint64, protocol::chat::v1::Message> messages;
};
