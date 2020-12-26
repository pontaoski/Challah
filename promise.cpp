#include "promise.hpp"

#include <QJSEngine>

void Promise::fufill(const QVariant &var)
{
	for (auto then : m_thens)
	{
		auto val = then.engine()->toScriptValue(var);
		then.call({val});
	}
}

void Promise::reject(const QVariant &var)
{
	for (auto except : m_excepts)
	{
		auto val = except.engine()->toScriptValue(var);
		except.call({val});
	}
}
