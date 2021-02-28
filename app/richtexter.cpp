#include "richtexter.hpp"

#include <QDebug>
#include <QEvent>
#include <QSet>
#include <QTextCursor>
#include <QKeyEvent>
#include <QKeySequence>

class TextFormatter::Private {
public:
	QTextDocument* parent = nullptr;
	QObject* field = nullptr;
	bool ignoreSignals = false;
	QSet<ITextEntityFormatter*> formatters;
	QList<QPair<QTextCursor,QString>> imageCursors;
};

class UnderlineFormatter : public ITextEntityFormatter {
public:
	UnderlineFormatter() {}
	~UnderlineFormatter() {}

	const QRegularExpression regexp = QRegularExpression("(__.*__)");

	QRegularExpression matches() const override {
		return regexp;
	}
	TextStyle styleFor(const QString&) const override {
		return TextStyle {
			CharacterStyle {
				{},
				{},
				true, // underline
				{},
				{},
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
	TextStyle styleFor(const QString&) const override {
		return TextStyle {
			CharacterStyle {
				{},
				{},
				{},
				true, // italic
				{},
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
	TextStyle styleFor(const QString&) const override {
		QImage img(128, 128, QImage::Format_ARGB32);
		img.fill(Qt::red);
		return TextStyle {
			img
		};
	}
};

class StrikethroughFormatter : public ITextEntityFormatter {
public:
	StrikethroughFormatter() {}
	~StrikethroughFormatter() {}

	const QRegularExpression regexp = QRegularExpression("(~~.*~~)");

	QRegularExpression matches() const override {
		return regexp;
	}
	TextStyle styleFor(const QString&) const override {
		return TextStyle {
			CharacterStyle {
				{},
				{},
				{},
				{},
				true, // strikethrough
			}
		};
	}
};

TextFormatter::TextFormatter(QTextDocument* parent, QObject* field)
{
	p = new Private;

	p->parent = parent;
	p->field = field;

	field->installEventFilter(this);

	registerFormatter(new UnderlineFormatter);
	registerFormatter(new ItalicFormatter);
	registerFormatter(new EmojiFormatter);
	registerFormatter(new StrikethroughFormatter);

	connect(parent, &QTextDocument::contentsChange, this, &TextFormatter::handleTextChanged);
}

TextFormatter::~TextFormatter()
{
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

	for (auto formatter : p->formatters) {
		auto matches = formatter->matches().globalMatch(plaintextBuffer);
		while (matches.hasNext()) {
			auto match = matches.next();
			auto word = match.captured(1);

			auto format = formatter->styleFor(word);

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
				if (style->strikethrough.has_value()) {
					fmt.setFontStrikeOut(*style->strikethrough);
				}
				if (style->font.has_value()) {
					fmt.setFont(*style->font);
				}

				cursor.mergeCharFormat(fmt);

			} else if (auto style = std::get_if<ImageStyle>(&format)) {

				auto img = *style;

				cursor.insertImage(img);

				QTextCursor newCursor(p->parent);
				newCursor.setPosition(match.capturedStart(1));
				newCursor.movePosition(QTextCursor::MoveOperation::Right, QTextCursor::MoveMode::KeepAnchor, 1);

				p->imageCursors << qMakePair(newCursor, word);

			}
		}
	}
	p->ignoreSignals = false;

}
