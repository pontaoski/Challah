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
		return QDateTime::fromMSecsSinceEpoch(d->messages[idx].created_at());
	case Roles::EditedAt:
		return QDateTime::fromMSecsSinceEpoch(d->messages[idx].created_at());
	case Roles::InReplyTo:
		return QString::number(d->messages[idx].in_reply_to());
	case Roles::Timestamp:
		return QDateTime::fromMSecsSinceEpoch(d->messages[idx].created_at()).toString("hh:mm");

	case Roles::OverrideAvatar: {
		const auto& overrides = d->messages[idx].overrides();

		return QString::fromStdString(overrides.avatar());
	}
	case Roles::OverrideName: {
		const auto& overrides = d->messages[idx].overrides();

		return QString::fromStdString(overrides.username());
	}

	case Roles::Author:
		return QString::number(d->messages[idx].author_id());

	case Roles::ContentType:
		switch (d->messages[idx].content().content_case()) {
		case protocol::chat::v1::Content::kTextMessage:
			return "textMessage";
		case protocol::chat::v1::Content::kEmbedMessage:
			return "embedMessage";
		case protocol::chat::v1::Content::kAttachmentMessage:
			return "filesMessage";
		default:
			return "unsupporteed";
		}

	case Roles::ContentText:
		return QString::fromStdString(d->messages[idx].content().text_message().content().text());
	case Roles::ContentEmbed:
		return conv(d->messages[idx].content().embed_message().embed());
	case Roles::ContentAttachments:
		return conv(d->messages[idx].content().attachment_message()).object();
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

void MessagesStore::newMessage(quint64 id, protocol::chat::v1::Message cont)
{
	d->messages[id] = cont;
	Q_EMIT keyAdded(QString::number(id));
}

void MessagesStore::deleteMessage(quint64 id)
{
	d->messages.remove(id);
	Q_EMIT keyRemoved(QString::number(id));
}

void MessagesStore::editMessage(quint64 id, protocol::chat::v1::FormattedText content)
{
	if (d->messages[id].content().content_case() != protocol::chat::v1::Content::kTextMessage) {
		return;
	}
	d->messages[id].mutable_content()->mutable_text_message()->mutable_content()->Swap(&content);
	Q_EMIT keyDataChanged(QString::number(id), {ContentText});
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
