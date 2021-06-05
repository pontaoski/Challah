#include <QUrl>
#include <QHostAddress>
#include <QHostInfo>

#include "resolver.h"

auto isIP(const QString& it)
{
	QHostAddress addr(it);
	return addr.protocol() == QAbstractSocket::IPv4Protocol ||
			addr.protocol() == QAbstractSocket::IPv6Protocol ||
			addr.protocol() == QAbstractSocket::AnyIPProtocol;
}

QIviPendingReply<QString> NameResolver::resolveURL(const QString& it)
{
	auto reply = QIviPendingReply<QString>();
	auto url = QUrl::fromUserInput(it);

	if (isIP(url.host())) {
		reply.setSuccess(url.toString());
	} else if (url.port() != -1) {
		reply.setSuccess(url.toString());
	}

	return reply;
}
