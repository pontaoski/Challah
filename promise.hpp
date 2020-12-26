#pragma once

#include <QJSValue>
#include <QVariant>

struct Promise
{
	Q_GADGET

	QList<QJSValue> m_thens;
	QList<QJSValue> m_excepts;

public:
	void fufill(const QVariant &variant);
	void reject(const QVariant &variant);

	Q_INVOKABLE Promise then(QJSValue val)
	{
		m_thens << val;

		return *this;
	}
	Q_INVOKABLE Promise except(QJSValue val)
	{
		m_excepts << val;

		return *this;
	}
};
