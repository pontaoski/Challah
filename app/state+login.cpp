#include <QQuickItem>
#include <QQmlProperty>

#include "state_p.h"

auto conn = ConnHelper::connect;

auto setProp(
	QObject* item,
	const QString& name,
	const QVariant& val
) {
	QQmlProperty prop(item, name, qmlContext(item));
	prop.write(val);
}

auto translate(
	const QString& source
) -> QString {
	QMap<QString,std::function<QString()>> data = {
		{"initial-choice", []() -> QString { return QObject::tr("Select a login method"); } },
		{"register", []() -> QString { return QObject::tr("Register"); } },
		{"login", []() -> QString { return QObject::tr("Login"); } },
		{"other-options", []() -> QString { return QObject::tr("Other Options"); } },
		{"reset-password", []() -> QString { return QObject::tr("Reset Password"); } },
		{"email", []() -> QString { return QObject::tr("Email:"); } },
		{"username", []() -> QString { return QObject::tr("Username:"); } },
		{"password", []() -> QString { return QObject::tr("Password:"); } },
	};

	if (data.contains(source)) {
		return data[source]();
	}

	return source;
}

auto translate(const std::string& source) -> QString { return translate(QString::fromStdString(source)); }

auto newComponent (
	QQmlEngine* engine,
	const QString& data,
	QObject* self
) -> QQmlComponent*
{
	auto component = new QQmlComponent(engine, self);
	component->setData(data.toLocal8Bit(), QUrl("state.cpp"));

	return component;
}

void State::createComponents()
{
	d->kirigamiHeadingComponent = newComponent(
		d->eng,
		R"(
import org.kde.kirigami 2.13 as Kirigami
Kirigami.Heading {}
		)",
		this
	);
	d->columnLayoutComponent = newComponent(
		d->eng,
		R"(
import QtQuick.Layouts 1.0
ColumnLayout {}
		)",
		this
	);
	d->buttonComponent = newComponent(
		d->eng,
		R"(
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.0
Button {}
		)",
		this
	);
	d->textFieldComponent = newComponent(
		d->eng,
		R"(
import QtQuick.Controls 2.12
import org.kde.kirigami 2.12 as Kirigami
TextField {}
		)",
		this
	);
	d->kirigamiFormLayoutComponent = newComponent(
		d->eng,
		R"(
import org.kde.kirigami 2.12 as Kirigami
Kirigami.FormLayout {}
		)",
		this
	);
	d->passwordFieldComponent = newComponent(
		d->eng,
		R"(
import QtQuick 2.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.12 as Kirigami
TextField {
	echoMode: TextInput.PasswordEchoOnEdit
}
		)",
		this
	);
}

void State::handleStep(protocol::auth::v1::AuthStep step)
{
	switch (step.step_case()) {
	case protocol::auth::v1::AuthStep::StepCase::kChoice: {
		auto& choice = step.choice();

		auto column = (QQuickItem*) d->columnLayoutComponent->create(qmlContext(this));

		d->kirigamiHeadingComponent->createWithInitialProperties(
			{
				{"text", translate(choice.title())},
				{"parent", QVariant::fromValue(column)}
			},
			qmlContext(this)
		);

		for (auto& opt : choice.options()) {
			auto button = d->buttonComponent->createWithInitialProperties(
				{
					{"text", translate(opt)},
					{"parent", QVariant::fromValue(column)},
				},
				qmlContext(this)
			);

			button->setObjectName(QStringLiteral("LoginScreen-Choice-%1").arg(QString::fromStdString(opt)));
			button->setParent(column);
			setProp(button, "Layout.alignment", Qt::AlignHCenter);
			setProp(button, "Layout.fillWidth", true);

			conn(button, SIGNAL(clicked()), [opt, this] {
				protocol::auth::v1::NextStepRequest req;
				req.set_allocated_choice(new protocol::auth::v1::NextStepRequest_Choice());
				req.mutable_choice()->set_choice(opt);

				d->sdk->continueAuthentication(req);
			});
		}

		Q_EMIT placeItem(column);
	}; break;
	case protocol::auth::v1::AuthStep::StepCase::kForm: {
		auto& form = step.form();

		QList<QPair<QQuickItem*,QString>> items;

		auto column = (QQuickItem*) d->columnLayoutComponent->create(qmlContext(this));

		auto heading = (QQuickItem*) d->kirigamiHeadingComponent->createWithInitialProperties(
			{
				{"text", translate(form.title())},
				{"parent", QVariant::fromValue(column)}
			},
			qmlContext(this)
		);
		Q_UNUSED(heading);

		auto formLayout = (QQuickItem*) d->kirigamiFormLayoutComponent->createWithInitialProperties(
			{
				{"parent", QVariant::fromValue(column)}
			},
			qmlContext(this)
		);

		for (auto& field : form.fields()) {
			QQuickItem* item;
			if (field.type() == "password" || field.type() == "new-password") {
				item = (QQuickItem*) d->passwordFieldComponent->createWithInitialProperties(
					{
						{"parent", QVariant::fromValue(formLayout)},
					},
					qmlContext(this)
				);
			} else {
				item = (QQuickItem*) d->textFieldComponent->createWithInitialProperties(
					{
						{"parent", QVariant::fromValue(formLayout)}
					},
					qmlContext(this)
				);
			}
			item->setObjectName(QStringLiteral("LoginScreen-%1-%2").arg(QString::fromStdString(field.type())).arg(QString::fromStdString(field.name())));
			setProp(item, "Kirigami.FormData.label", translate(field.name()));
			if (field.type() == "email") {
				setProp(item, "placeholderText", tr("address@email.com"));
			}

			items << qMakePair(item, QString::fromStdString(field.type()));
		}

		auto button = (QQuickItem*) d->buttonComponent->createWithInitialProperties(
			{
				{"text", tr("Submit")},
				{"parent", QVariant::fromValue(column)},
			},
			qmlContext(this)
		);
		button->setObjectName(QStringLiteral("LoginScreen-SubmitButton"));

		setProp(button, "Layout.alignment", Qt::AlignRight);

		conn(button, SIGNAL(clicked()), [items, this] {
			protocol::auth::v1::NextStepRequest req;
			req.set_allocated_form(new protocol::auth::v1::NextStepRequest_Form);

			for (auto& item : items) {
				auto field = req.mutable_form()->mutable_fields()->Add();

				if (QList<QString>{"email", "username"}.contains(item.second)) {
					field->set_string(item.first->property("text").toString().toStdString());
				} else if (QList<QString>{"password", "new-password"}.contains(item.second)) {
					field->set_bytes(item.first->property("text").toString().toStdString());
				}
			}
			d->sdk->continueAuthentication(req);
		});

		Q_EMIT placeItem(column);
	}; break;
	case protocol::auth::v1::AuthStep::StepCase::kWaiting: {
		auto& waiting = step.waiting();

		auto column = (QQuickItem*) d->columnLayoutComponent->create(qmlContext(this));

		auto title = (QQuickItem*) d->kirigamiHeadingComponent->createWithInitialProperties(
			{
				{"text", translate(waiting.title())},
				{"parent", QVariant::fromValue(column)},
			},
			qmlContext(this)
		);

		auto subtitle = (QQuickItem*) d->kirigamiHeadingComponent->createWithInitialProperties(
			{
				{"text", translate(waiting.description())},
				{"parent", QVariant::fromValue(column)},
				{"level", 3}
			},
			qmlContext(this)
		);

		setProp(title, "Layout.fillWidth", true);
		setProp(title, "wrapMode", Qt::TextWordWrap);
		setProp(subtitle, "Layout.fillWidth", true);
		setProp(subtitle, "wrapMode", Qt::TextWordWrap);
	}; break;
	case protocol::auth::v1::AuthStep::StepCase::kSession:
		break;
	case protocol::auth::v1::AuthStep::StepCase::STEP_NOT_SET:
		break;
	}
}
