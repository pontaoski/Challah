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
