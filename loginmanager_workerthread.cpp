#include "loginmanager.hpp"
#include "loginmanager_p.hpp"

void LoginManager::runWork()
{
	auto client = State::instance()->client;

	protocol::auth::v1::BeginAuthResponse beginAuth;

	{
		ClientContext ctx;
		client->authenticate(ctx);

		if (!checkStatus(client->authKit->BeginAuth(&ctx, google::protobuf::Empty{}, &beginAuth))) {
			QCoreApplication::postEvent(this, new ErrorEvent);
			return;
		}
	}

	d->authIDMutex.lock();
	d->authID = QString::fromStdString(beginAuth.auth_id());
	d->authIDMutex.unlock();

	ClientContext ctx;
	client->authenticate(ctx);

	protocol::auth::v1::AuthStep step;

	{
		ClientContext ctx2;
		client->authenticate(ctx2);

		protocol::auth::v1::NextStepRequest req;
		req.set_auth_id(d->authID.toStdString());

		if (!checkStatus(client->authKit->NextStep(&ctx2, req, &step))) {
			QCoreApplication::postEvent(this, new ErrorEvent);
			return;
		}

		QCoreApplication::postEvent(this, new StepEvent(step));
	}

	protocol::auth::v1::StreamStepsRequest streamReq;
	streamReq.set_auth_id(beginAuth.auth_id());

	auto stepStream = client->authKit->StreamSteps(&ctx, streamReq);
	stepStream->WaitForInitialMetadata();

	while (stepStream->Read(&step)) {
		if (step.has_session()) {
			QCoreApplication::postEvent(this, new SessionEvent(step.session()));
			return;
		}

		QCoreApplication::postEvent(this, new StepEvent(step));
	}

	QCoreApplication::postEvent(this, new ErrorEvent);
}
