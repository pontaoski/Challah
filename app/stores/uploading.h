#pragma once

#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QUrlQuery>

#include "state.h"

#include "coroutine_integration_network.h"

inline Croutons::FutureResult<QString> uploadFile(State* state, QString host, QUrl url)
{
	QHttpMultiPart *mp = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	const auto furl = url.isLocalFile() ? url.toLocalFile() : url.toString();
	QFile* file(new QFile(furl));
	file->open(QIODevice::ReadOnly);

	const auto fname = QString::fromLocal8Bit(QUrl::toPercentEncoding(url.fileName()));

	QHttpPart filePart;
	filePart.setBodyDevice(file);
	filePart.setHeader(
		QNetworkRequest::ContentDispositionHeader,
		QString("form-data; name=\"file\"; filename=\"%1\"").arg(fname));
	QMimeDatabase db;
	const auto mt = db.mimeTypeForUrl(url);
	filePart.setHeader(QNetworkRequest::ContentTypeHeader, mt.name());

	mp->append(filePart);

	QUrlQuery query;
	query.addQueryItem("filename", url.fileName());
	query.addQueryItem("contentType", QMimeDatabase().mimeTypeForFile(furl).name());

	auto c = co_await state->api()->clientForHomeserver(host);

	QUrl reqURL;

	if (c->homeserver().contains("//")) {
		reqURL = QUrl::fromUserInput(c->homeserver());
	} else {
		reqURL.setHost(c->homeserver());
		reqURL.setPort(2289);
		reqURL.setScheme("https");
	}

	reqURL.setPath("/_harmony/media/upload");
	reqURL.setQuery(query);

	QNetworkRequest req(reqURL);
	req.setRawHeader("Authorization", QByteArray::fromStdString(c->session()));
	QNetworkAccessManager nam;

	const auto reply = co_await nam.post(req, mp);
	if (reply->error() != QNetworkReply::NoError) {
		co_return Error{reply->errorString()};
	}

	const auto body = reply->readAll();
	const auto id = QJsonDocument::fromJson(body)["id"].toString();

	co_return id;
}

inline Croutons::FutureResult<QStringList> uploadFiles(State* state, QString host, QList<QUrl> urls)
{
	QStringList ids;

	for (const auto& url : urls) {
		auto id = co_await uploadFile(state, host, url);

		if (!id.ok()) {
			co_return Error{id.error()};
		}

		ids << id.value();
	}

	co_return ids;
}
