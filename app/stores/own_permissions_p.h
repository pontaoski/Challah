#pragma once

#include "own_permissions.h"

struct Node
{
	QString homeserver;
	quint64 guild = 0;
	quint64 channel = 0;
	QString node;

	static auto from(const QVariant& variant) -> Node
	{
		auto li = variant.value<QVariantList>();

		return Node {
			li[0].toString(),
			li[1].toString().toULongLong(),
			li[2].toString().toULongLong(),
			li[3].toString(),
		};
	}
	auto into() -> QVariant
	{
		return QVariantList{QString::number(guild), QString::number(channel), node};
	}

	bool operator<(const Node& rhs) const
	{
		auto lhs = this;

		return lhs->homeserver < rhs.homeserver
			|| lhs->guild < rhs.guild
			|| lhs->channel < rhs.channel
			|| lhs->node < rhs.node;
	}

	bool operator==(const Node& rhs) const
	{
		auto lhs = this;

		return lhs->homeserver == rhs.homeserver
			&& lhs->guild == rhs.guild
			&& lhs->channel == rhs.guild
			&& lhs->node == rhs.node;
	}
};

struct OwnPermissionsStore::Private
{
	QMap<Node, bool> data;
	QList<Node> fetching;
};
