#pragma once

#include <QEvent>
#include <QMutex>
#include <QQmlComponent>

#include "loginmanager.hpp"
#include "auth/v1/auth.grpc.pb.h"
#include "util.hpp"
#include "state.hpp"

using grpc::ClientContext;

class ErrorEvent : public QEvent
{
public:
	ErrorEvent() : QEvent(QEvent::User) {}
};

using StepEvent = CarrierEvent<30, protocol::auth::v1::AuthStep>;
using SessionEvent = CarrierEvent<31, protocol::auth::v1::Session>;

struct LoginManager::Private
{
	QMutex authIDMutex;
	QString authID;

	QQmlComponent* columnLayoutComponent;
	QQmlComponent* kirigamiHeadingComponent;
	QQmlComponent* buttonComponent;
	QQmlComponent* textFieldComponent;
	QQmlComponent* kirigamiFormLayoutComponent;
	QQmlComponent* passwordFieldComponent;
};

class ConnHelper : public QObject {
	Q_OBJECT

	std::function<void()> func;

public:
	Q_SLOT void trigger() { func(); }
	ConnHelper(std::function<void()>&& fun, QObject* parent = nullptr) : QObject(parent), func(std::move(fun))
	{

	}

	static QMetaObject::Connection connect(
		QObject* sender,
		const char* signal,
		std::function<void()>&& func
	) {
		if (!sender) return QMetaObject::Connection();

		return QObject::connect(sender, signal, new ConnHelper(std::move(func), sender), SLOT(trigger()));
	}
};
