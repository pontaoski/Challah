// SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "util.hpp"
#include "state.hpp"

bool checkStatusImpl(const char* file, int line, grpc::Status status) {
	if (!status.ok()) {
		qDebug() << "There was an error ( code" << status.error_code() << ")" << "at" << file << ":" << line;
		qDebug() << "\tDetails" << status.error_details().c_str();
		qDebug() << "\tMessage" << status.error_message().c_str();
		return false;
	}
	return true;
}

void runOnMainThread(std::function<void ()> f)
{
	QCoreApplication::postEvent(State::instance(), new ExecuteEvent(f));
}
