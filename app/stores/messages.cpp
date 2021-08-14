#include <QJsonDocument>
#include <QJsonObject>

#include "messages_p.h"
#include "messages_model.h"

enum Roles {
	// universal
	Overrides,
	CreatedAt,
	EditedAt,
	InReplyTo,
	Author,
	Timestamp,

	OverrideAvatar,
	OverrideName,

	// type tag
	ContentType,

	// specific stuff
	ContentText,

	ContentEmbed,

	ContentAttachments,
};

MessagesStore::MessagesStore(MessagesModel* parent, SDK::Client* c) : ChallahAbstractRelationalModel(parent), c(c), p(parent), d(new Private)
{
}

MessagesStore::~MessagesStore()
{
}

template<typename T>
auto conv(const T& it) {
	std::string jsonified;
	google::protobuf::util::MessageToJsonString(it, &jsonified, google::protobuf::util::JsonPrintOptions{});
	auto document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonified));
	return document;
}

QVariant MessagesStore::data(const QVariant& key, int role)
{
	if (!checkKey(key)) {
		return QVariant();
	}

	auto idx = key.toString().toULongLong();

	switch (role) {
	case Roles::Overrides:
		return conv(d->messages[idx].overrides());
	case Roles::CreatedAt:
		return QDateTime::fromTime_t(d->messages[idx].created_at().seconds());
	case Roles::EditedAt:
		return QDateTime::fromTime_t(d->messages[idx].created_at().seconds());
	case Roles::InReplyTo:
		return QString::number(d->messages[idx].in_reply_to());
	case Roles::Timestamp:
		return QDateTime::fromTime_t(d->messages[idx].created_at().seconds()).toString("hh:mm");

	case Roles::OverrideAvatar: {
		const auto& overrides = d->messages[idx].overrides();

		return QString::fromStdString(overrides.avatar());
	}
	case Roles::OverrideName: {
		const auto& overrides = d->messages[idx].overrides();

		return QString::fromStdString(overrides.name());
	}

	case Roles::Author:
		return QString::number(d->messages[idx].author_id());

	case Roles::ContentType:
		switch (d->messages[idx].content().content_case()) {
		case protocol::harmonytypes::v1::Content::kTextMessage:
			return "textMessage";
		case protocol::harmonytypes::v1::Content::kEmbedMessage:
			return "embedMessage";
		case protocol::harmonytypes::v1::Content::kFilesMessage:
			return "filesMessage";
		default:
			return "unsupporteed";
		}

	case Roles::ContentText:
		return QString::fromStdString(d->messages[idx].content().text_message().content());
	case Roles::ContentEmbed:
		return conv(d->messages[idx].content().embed_message().embeds());
	case Roles::ContentAttachments:
		return conv(d->messages[idx].content().files_message()).object();
	}

	return QVariant();
}

bool MessagesStore::checkKey(const QVariant& key)
{
	return d->messages.contains(key.toString().toULongLong());
}

bool MessagesStore::canFetchKey(const QVariant& key)
{
	Q_UNUSED(key)
	return false;
}

void MessagesStore::fetchKey(const QVariant& key)
{
	Q_UNUSED(key)
}

void MessagesStore::newMessage(quint64 id, protocol::harmonytypes::v1::Message cont)
{
	d->messages[id] = cont;
	Q_EMIT keyAdded(QString::number(id));
}

QHash<int, QByteArray> MessagesStore::roleNames()
{
	return {
		{ Overrides, "overrides" },
		{ CreatedAt, "createdAt" },
		{ EditedAt,  "editedAt" },
		{ InReplyTo, "inReplyTo" },
		{ Author,    "author" },
		{ Timestamp, "timestamp" },

		{ OverrideAvatar, "overrideAvatar" },
		{ OverrideName, "overrideName" },

		{ ContentType, "contentType" },

		{ ContentText, "contentText" },
		{ ContentEmbed, "contentEmbed" },
		{ ContentAttachments, "contentAttachments" },
	};
}
