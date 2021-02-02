#include "loginmanager.hpp"
#include "loginmanager_p.hpp"
#include "util.hpp"

#define theHeaders {{"Authorization", client->userToken}}

void LoginManager::runWork()
{
	auto client = State::instance()->client;

	auto result = client->authKit->BeginAuth(google::protobuf::Empty{}, theHeaders);
	if (!resultOk(result)) {
		QCoreApplication::postEvent(this, new ErrorEvent);
	}
	auto resp = unwrap(result);

	d->authIDMutex.lock();
	d->authID = QString::fromStdString(resp.auth_id());
	d->authIDMutex.unlock();

	protocol::auth::v1::NextStepRequest req2;
	req2.set_auth_id(d->authID.toStdString());
	auto result2 = client->authKit->NextStep(req2, theHeaders);
	if (!resultOk(result2)) {
		QCoreApplication::postEvent(this, new ErrorEvent);
		return;
	}
	auto resp2 = unwrap(result2);

	QCoreApplication::postEvent(this, new StepEvent(resp2));

	protocol::auth::v1::StreamStepsRequest streamReq;
	streamReq.set_auth_id(resp.auth_id());

	auto stepStream = client->authKit->StreamSteps(streamReq, theHeaders);
	connect(stepStream, &Receive__protocol_auth_v1_AuthStep__Stream::receivedMessage, [=](const protocol::auth::v1::AuthStep& step) {
		qDebug() << "mu";
		if (step.has_session()) {
			QCoreApplication::postEvent(this, new SessionEvent(step.session()));
			stepStream->deleteLater();
		}

		QCoreApplication::postEvent(this, new StepEvent(step));
	});
	connect(stepStream, &QWebSocket::disconnected, [=] {
		stepStream->deleteLater();
	});
}
