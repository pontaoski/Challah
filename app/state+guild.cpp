#include <QSettings>
#include <QQmlEngine>

#include "state_p.h"

FutureBase State::createGuild(QString name)
{
	protocol::chat::v1::CreateGuildRequest req;
	req.set_name(name.toStdString());
	auto result = co_await d->sdk->chatKit()->CreateGuild(req);
	co_return result.ok();
}

FutureBase State::joinGuild(QString name)
{
	if (name.contains("//")) {
		auto url = QUrl::fromUserInput(name);

		auto client = co_await d->sdk->clientForHomeserver(url.toString());

		protocol::chat::v1::JoinGuildRequest req;
		req.set_invite_id(url.path().mid(1).toStdString());

		co_return (co_await client->chatKit()->JoinGuild(req)).ok();
	} else {
		protocol::chat::v1::JoinGuildRequest req;
		req.set_invite_id(name.toStdString());
		co_return (co_await d->sdk->chatKit()->JoinGuild(req)).ok();
	}
}
