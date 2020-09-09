#include "util.hpp"

bool checkStatus(grpc::Status status) {
	if (!status.ok()) {
		qDebug() << "There was an error ( code" << status.error_code() << ")";
		qDebug() << "\tDetails" << status.error_details().c_str();
		qDebug() << "\tMessage" << status.error_message().c_str();
		return false;
	}
	return true;
}