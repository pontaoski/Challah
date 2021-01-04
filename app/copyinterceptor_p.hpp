#pragma once

#include "copyinterceptor.hpp"

class CopyInterceptor::Private
{
public:
	QJSValue copy;
	QJSValue paste;

	QJSValue generateJSValueFromClipboard();
};
