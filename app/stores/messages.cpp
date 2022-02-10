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

	ContentPhotos,
};

MessagesStore::MessagesStore(MessagesModel* parent, State* s, QString host) : ChallahAbstractRelationalModel(parent), s(s), host(host), p(parent), d(new Private)
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

	auto idx = MessageID::fromString(key.toString());

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
		case protocol::chat::v1::Content::kPhotoMessage:
			return "photosMessage";
		default:
			return "unsupporteed";
		}

	case Roles::ContentText:
		return conv(d->messages[idx].content().text_message().content()).object();
	case Roles::ContentEmbed:
		return conv(d->messages[idx].content().embed_message()).object();
	case Roles::ContentAttachments:
		return conv(d->messages[idx].content().attachment_message()).object();
	case Roles::ContentPhotos:
		return conv(d->messages[idx].content().photo_message()).object();
	}

	return QVariant();
}

bool MessagesStore::checkKey(const QVariant& key)
{
	return d->messages.contains(MessageID::fromString(key.toString()));
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
	const auto mid = MessageID { MessageID::Remote, id };
	d->messages[mid] = cont;
	Q_EMIT keyAdded(mid.toString());
}

void MessagesStore::deleteMessage(quint64 id)
{
	const auto mid = MessageID { MessageID::Remote, id };
	d->messages.remove(mid);
	Q_EMIT keyRemoved(mid.toString());
}

void MessagesStore::editMessage(quint64 id, protocol::chat::v1::FormattedText content)
{
	const auto mid = MessageID { MessageID::Remote, id };
	if (d->messages[mid].content().content_case() != protocol::chat::v1::Content::kTextMessage) {
		return;
	}
	d->messages[mid].mutable_content()->mutable_text_message()->mutable_content()->Swap(&content);
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
		{ ContentPhotos, "contentPhotos" },
	};
}
