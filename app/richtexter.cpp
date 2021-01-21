#include "richtexter.hpp"
#include "qtextformat.h"
#include "state.hpp"

#include <QDebug>
#include <QEvent>
#include <QSet>
#include <QTextCursor>
#include <QKeyEvent>
#include <QKeySequence>
#include <QTextBlock>

class TextFormatter::Private {
public:
	QTextDocument* parent = nullptr;
	QObject* field = nullptr;
	bool ignoreSignals = false;
	QSet<ITextEntityFormatter*> formatters;
	QList<QPair<QTextCursor,QString>> imageCursors;
	QString homeserver;
};

class UnderlineFormatter : public ITextEntityFormatter {
public:
	UnderlineFormatter() {}
	~UnderlineFormatter() {}

	const QRegularExpression regexp = QRegularExpression("(__.*__)");

	QRegularExpression matches() const override {
		return regexp;
	}
	TextStyle styleFor(const QString&, const FormatContext&) const override {
		return TextStyle {
			CharacterStyle {
				.underline = true
			}
		};
	}
};

class ItalicFormatter : public ITextEntityFormatter {
public:
	ItalicFormatter() {}
	~ItalicFormatter() {}

	const QRegularExpression regexp = QRegularExpression("(\\*.*\\*)");

	QRegularExpression matches() const override {
		return regexp;
	}
	TextStyle styleFor(const QString&, const FormatContext&) const override {
		return TextStyle {
			CharacterStyle {
				.italic = true
			}
		};
	}
};

class EmojiFormatter : public ITextEntityFormatter {
public:
	EmojiFormatter() {}
	~EmojiFormatter() {}

	const QRegularExpression regexp = QRegularExpression("(<:.*?:>)");

	QRegularExpression matches() const override {
		return regexp;
	}
	TextStyle styleFor(const QString& emoji, const FormatContext& ctx) const override {
		return TextStyle {
			ImageStyle {
				.source = State::instance()->transformHMCURL(emoji.mid(2).chopped(2), ctx.homeserver),
				.size = QSizeF(22, 22)
			}
		};
	}
};

TextFormatter::TextFormatter(QTextDocument* parent, const QString& homeserver, QObject* field)
{
	s_instances[field] = this;

	p = new Private;

	p->parent = parent;
	p->field = field;
	p->homeserver = homeserver;

	field->installEventFilter(this);

	registerFormatter(new UnderlineFormatter);
	registerFormatter(new ItalicFormatter);
	registerFormatter(new EmojiFormatter);

	connect(parent, &QTextDocument::contentsChange, this, &TextFormatter::handleTextChanged);
}

TextFormatter::~TextFormatter()
{
	s_instances.remove(parent());

	delete p;
}

void TextFormatter::registerFormatter(ITextEntityFormatter* formatter)
{
	p->formatters.insert(formatter);
}

void TextFormatter::removeTextFormatter(ITextEntityFormatter* formatter)
{
	p->formatters.remove(formatter);
}

bool TextFormatter::eventFilter(QObject *object, QEvent *event)
{
	if (auto ev = dynamic_cast<QKeyEvent*>(event)) {
		if (ev == QKeySequence::Copy) {
			return false;
		}
	}
	return false;
}

QString TextFormatter::plaintext()
{
	return plaintext(0, p->parent->characterCount());
}

QMap<QObject*,TextFormatter*> TextFormatter::s_instances;

QString TextFormatter::plaintext(int from, int to)
{
	QString ret;
	ret.reserve(to - from);

	int offsetFrom = from, offsetTo = to;
	int pos = 0;
	bool pastFrom = false, pastTo = false;

	for (int i = 0; i < p->parent->blockCount(); i++) {
		auto currentBlock = p->parent->findBlockByNumber(i);

		QTextBlock::iterator it;
		for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
			QTextFragment currentFragment = it.fragment();

			if (currentFragment.isValid()) {
				if (currentFragment.charFormat().isImageFormat()) {
					ret += currentFragment.charFormat().toImageFormat().anchorHref();
					currentFragment.charFormat().toImageFormat().anchorHref().length();
					pos += currentFragment.text().length();
					if (!pastFrom) {
						offsetFrom += currentFragment.charFormat().toImageFormat().anchorHref().length();
					}
					if (!pastTo) {
						offsetTo += currentFragment.charFormat().toImageFormat().anchorHref().length();
					}
				} else {
					ret += currentFragment.text();
					pos += currentFragment.text().length();
				}
			}

			if (pos > offsetFrom) {
				pastFrom = true;
			}
			if (pos > offsetTo) {
				pastTo = true;
			}
		}
	}

	return ret.mid(offsetFrom, offsetTo - offsetFrom);
}

void TextFormatter::handleTextChanged(int position, int charsRemoved, int charsAdded)
{
	if (p->ignoreSignals) return;

	p->ignoreSignals = true;

	QTextCursor curs(p->parent);
	curs.setPosition(0);
	curs.movePosition(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::KeepAnchor);

	QTextCursor prev;
	for (int i = 0; i < p->parent->characterCount(); i++) {
		auto rune = p->parent->characterAt(i);

		if (rune == QChar::ObjectReplacementCharacter) {
			continue;
		}

		QTextCursor clearer(p->parent);
		clearer.setPosition(i);
		clearer.movePosition(QTextCursor::MoveOperation::Right, QTextCursor::MoveMode::KeepAnchor, 1);
		clearer.setCharFormat(QTextCharFormat());
	}

	auto plaintextBuffer = curs.selectedText();

	auto ctx = FormatContext {
		.homeserver = p->homeserver
	};

	for (auto formatter : p->formatters) {
		auto matches = formatter->matches().globalMatch(plaintextBuffer);
		while (matches.hasNext()) {
			auto match = matches.next();
			auto word = match.captured(1);

			auto format = formatter->styleFor(word, ctx);

			QTextCursor cursor(p->parent);
			cursor.setPosition(match.capturedStart(1));
			cursor.movePosition(QTextCursor::MoveOperation::Right, QTextCursor::MoveMode::KeepAnchor, match.capturedLength(1));

			if (auto style = std::get_if<CharacterStyle>(&format)) {

				QTextCharFormat fmt;

				if (style->weight.has_value()) {
					fmt.setFontWeight(*style->weight);
				}
				if (style->italic.has_value()) {
					fmt.setFontItalic(*style->italic);
				}
				if (style->underline.has_value()) {
					fmt.setFontUnderline(*style->underline);
				}
				if (style->font.has_value()) {
					fmt.setFont(*style->font);
				}

				cursor.mergeCharFormat(fmt);

			} else if (auto style = std::get_if<ImageStyle>(&format)) {

				auto img = *style;

				cursor.insertImage(img.source);

				QTextCursor newCursor(p->parent);
				newCursor.setPosition(match.capturedStart(1));
				newCursor.movePosition(QTextCursor::MoveOperation::Right, QTextCursor::MoveMode::KeepAnchor, 1);

				if (img.size.has_value()) {
					QTextImageFormat format;
					format.setHeight(img.size.value().height());
					format.setWidth(img.size.value().width());
					format.setName(img.source);
					format.setAnchorHref(word);
					format.setVerticalAlignment(QTextCharFormat::AlignBottom);

					newCursor.setCharFormat(format);
				}

				p->imageCursors << qMakePair(newCursor, word);

			}
		}
	}
	p->ignoreSignals = false;

}
