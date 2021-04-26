#include <QQmlListProperty>
#include "conditional.h"

void Conditional::componentComplete()
{
	applyChanges();
	connect(this, &Conditional::predChanged, [=] { applyChanges(); });
	connect(this, &Conditional::succChanged, [=] { applyChanges(); });
	connect(this, &Conditional::ifTrueChanged, [=] { m_ifTrueDirty = true; applyChanges(); });
	connect(this, &Conditional::ifFalseChanged, [=] { m_ifFalseDirty = true; applyChanges(); });
	connect(this, &Conditional::valueChanged, [=] { applyChanges(); });
}

void Conditional::applyChanges()
{
	auto obj = qobject_cast<QQuickItem*>(parent());
	if (!obj) {
		qFatal("Conditional must be a child of a QQuickItem*");
	}

	if (m_inst) {
		if (m_previousVal != m_value) {
			m_inst->setParentItem(nullptr);
			m_inst->deleteLater();
			m_inst = nullptr;
		}
	}

	if (m_inst == nullptr) {
		if (m_value) {
			m_inst = qobject_cast<QQuickItem*>(m_ifTrue->create());
		} else {
			m_inst = qobject_cast<QQuickItem*>(m_ifFalse->create());
		}
	} else {
		if (m_value) {
			if (m_ifTrueDirty) {
				m_inst = qobject_cast<QQuickItem*>(m_ifTrue->create());
			}
		} else {
			if (m_ifFalseDirty) {
				m_inst = qobject_cast<QQuickItem*>(m_ifFalse->create());
			}
		}
	}

	m_ifFalseDirty = false;
	m_ifTrueDirty = false;
	m_previousVal = m_value;

	m_inst->setParent(this);
	m_inst->setParentItem(obj);
	m_inst->stackAfter(m_pred);
	m_inst->stackBefore(m_succ);
}
