#include <QLocale>
#include <QDebug>
#include <QQuickTextDocument>
#include <QJsonDocument>
#include <QTextCursor>
#include <QQuickItem>
#include <QQmlProperty>
#include <QQmlEngine>
#include <QFontDatabase>
#include <QImageReader>

#include <chat/v1/messages.pb.h>
#include <google/protobuf/util/json_util.h>

#include "utils.h"
#include "state.h"

#include "coroutine_integration_network.h"

struct Utils::Private
{
	QNetworkAccessManager nam;
};

Utils::Utils(QObject* parent) : QObject(parent), d(new Private)
{
	qWarning() << "new utils" << metaObject()->className();
}

Utils::~Utils()
{

}

QString Utils::formattedSize(int size)
{
	return QLocale().formattedDataSize(size, 1);
}

QString operator"" _qs (const char* s, std::size_t len)
{
	return QString::fromUtf8( s, len );
}

void Utils::formatDocument(State* s, QQuickTextDocument* txt, QQuickItem* field, QJsonObject obj, bool hideLastChar)
{
	using namespace protocol::chat::v1;

	const auto data = QJsonDocument(obj).toJson();

	protocol::chat::v1::FormattedText fmt;
	const auto status = google::protobuf::util::JsonStringToMessage(data.constData(), &fmt);
	if (!status.ok()) {
		auto s = status.ToString();
	}

	auto doku = txt->textDocument();
	QTextCursor curs(doku);

	QColor linkColor = QQmlProperty(field, "Kirigami.Theme.linkColor", qmlContext(field)).read().value<QColor>();
	QColor negColor = QQmlProperty(field, "Kirigami.Theme.negativeTextColor", qmlContext(field)).read().value<QColor>();
	QColor warColor = QQmlProperty(field, "Kirigami.Theme.neutralTextColor", qmlContext(field)).read().value<QColor>();
	QColor posColor = QQmlProperty(field, "Kirigami.Theme.positiveTextColor", qmlContext(field)).read().value<QColor>();
	const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

	if (hideLastChar) {
		QTextCursor curs(doku);

		curs.movePosition(QTextCursor::End);
		curs.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

		QTextCharFormat cfmt;
		cfmt.setForeground(Qt::transparent);

		curs.mergeCharFormat(cfmt);
	}

	for (const auto& entity : fmt.format()) {
		QTextCharFormat cfmt;
		bool custom = false;

		curs.setPosition(entity.start(), QTextCursor::MoveAnchor);
		curs.setPosition(entity.start() + entity.length(), QTextCursor::KeepAnchor);

		switch (entity.format_case()) {
		case Format::kBold:
			cfmt.setFontWeight(QFont::Bold);
			break;
		case Format::kItalic:
			cfmt.setFontItalic(true);
			break;
		case Format::kUnderline:
			cfmt.setFontUnderline(true);
			break;
		case Format::kMonospace:
			cfmt.setFont(fixedFont);
			break;
		case Format::kSuperscript:
			cfmt.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
			break;
		case Format::kSubscript:
			cfmt.setVerticalAlignment(QTextCharFormat::AlignSubScript);
			break;
		case Format::kCodeBlock:
			cfmt.setFont(fixedFont);
			break;
		case Format::kUserMention:
			cfmt.setForeground(linkColor);
			cfmt.setAnchor(true);
			cfmt.setAnchorHref("user:%1"_qs.arg(entity.user_mention().user_id()));
			break;
		case Format::kRoleMention:
			cfmt.setForeground(linkColor);
			cfmt.setAnchor(true);
			cfmt.setAnchorHref("role:%1"_qs.arg(entity.role_mention().role_id()));
			break;
		case Format::kChannelMention:
			cfmt.setForeground(linkColor);
			cfmt.setAnchor(true);
			cfmt.setAnchorHref("channel:%1"_qs.arg(entity.channel_mention().channel_id()));
			break;
		case Format::kGuildMention:
			cfmt.setForeground(linkColor);
			cfmt.setAnchor(true);
			cfmt.setAnchorHref(
				"guild:%1 %2"_qs.arg(
					entity.guild_mention().guild_id()
				).arg(
					QString::fromStdString(
						entity.guild_mention().homeserver())));
			break;
		case Format::kEmoji: {
			custom = true;
			const auto emoji = entity.emoji();
			const auto hmc = QString::fromStdString(emoji.image_hmc());

			const auto murl = s->mediaURL(hmc, s->api()->clientForHomeserver("local").result()->homeserver());
			const auto req = QNetworkRequest(murl);
			const auto future = Croutons::transformer<QNetworkReply*>::transform(d->nam.get(req));

			future.then([curs, murl](QNetworkReply* reply) mutable {
				if (reply->error() != QNetworkReply::NoError) {
					return;
				}

				const auto format = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLocal8Bit();
				QImageReader reader(reply, format);
				const auto img = reader.read().scaled(22, 22);

				img.save("/tmp/" + QUrl::toPercentEncoding(murl));
				curs.insertImage(img, QUrl::fromLocalFile("/tmp/" + QUrl::toPercentEncoding(murl)).toString());
			});

			break;
		}
		case Format::kColor:
			switch (entity.color().kind()) {
			case Format::Color::KIND_DIM_UNSPECIFIED:
				break;
			case Format::Color::KIND_BRIGHT:
				break;
			case Format::Color::KIND_NEGATIVE:
				cfmt.setForeground(negColor);
				break;
			case Format::Color::KIND_POSITIVE:
				cfmt.setForeground(posColor);
				break;
			case Format::Color::KIND_INFO:
				cfmt.setForeground(QColor("#3daee9"));
				break;
			case Format::Color::KIND_WARNING:
				cfmt.setForeground(warColor);
				break;
			default:
				break;
			}
			break;
		case Format::kLocalization:
			cfmt.setForeground(QColor("#ff0000"));
			break;
		default:
			break;
		}

		if (!custom) {
			curs.mergeCharFormat(cfmt);
		}
	}
}

QString Utils::naturalList(const QStringList& list)
{
	return QLocale().createSeparatedList(list);
}
