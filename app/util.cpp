// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "util.hpp"
#include "state.hpp"

void runOnMainThread(std::function<void ()> f)
{
	QCoreApplication::postEvent(State::instance(), new ExecuteEvent(f));
}
