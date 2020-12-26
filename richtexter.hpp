#pragma once

#include <QRegularExpression>
#include <QTextCharFormat>
#include <QImage>
#include <QTextDocument>

#include <variant>
#include <optional>

using QOptionalFont = std::optional<QFont>;
using OptionalFontWeight = std::optional<int>;
using OptionalBoolean = std::optional<bool>;

struct CharacterStyle {
	QOptionalFont font;
	OptionalFontWeight weight;
	OptionalBoolean underline;
	OptionalBoolean italic;
};

using ImageStyle = QImage;

using TextStyle = std::variant<ImageStyle,CharacterStyle>;

class ITextEntityFormatter {
public:
	virtual ~ITextEntityFormatter() { };
	virtual QRegularExpression matches() const = 0;
	virtual TextStyle styleFor(const QString&) const = 0;
};

class TextFormatter : public QObject {
	Q_OBJECT

public:
	explicit TextFormatter(QTextDocument* parent, QObject* field);
	~TextFormatter();

	void registerFormatter(ITextEntityFormatter* formatter);
	void removeTextFormatter(ITextEntityFormatter* formatter);

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

private:
	class Private;
	Private* p;

	void handleTextChanged(int position, int charsRemoved, int charsAdded);
};
