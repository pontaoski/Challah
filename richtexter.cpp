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
	QString plaintextBuffer = QString();
	QSet<ITextEntityFormatter*> formatters;
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
	TextStyle styleFor(const QString&) const override {
		return TextStyle {
			CharacterStyle {
				.italic = true
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
			qDebug() << "Intercepted copy event!";
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
	curs.setCharFormat(QTextCharFormat());

	p->plaintextBuffer = curs.selectedText();

	for (auto formatter : p->formatters) {
		auto matches = formatter->matches().globalMatch(p->plaintextBuffer);
		while (matches.hasNext()) {
			auto match = matches.next();
			auto word = match.captured(1);

			auto format = formatter->styleFor(word);

			QTextCursor cursor(p->parent);
			cursor.setPosition(match.capturedStart(1));
			cursor.movePosition(QTextCursor::MoveOperation::Right, QTextCursor::MoveMode::KeepAnchor, match.capturedLength(1));

			if (std::holds_alternative<CharacterStyle>(format)) {
				auto style = std::get<CharacterStyle>(format);

				QTextCharFormat fmt;

				if (style.weight.has_value()) {
					fmt.setFontWeight(style.weight.value());
				}
				if (style.italic.has_value()) {
					fmt.setFontItalic(style.italic.value());
				}
				if (style.underline.has_value()) {
					fmt.setFontUnderline(style.underline.value());
				}
				if (style.font.has_value()) {
					fmt.setFont(style.font.value());
				}

				cursor.mergeCharFormat(fmt);
			}
		}
	}
	p->ignoreSignals = false;

}
