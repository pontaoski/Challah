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

struct ImageStyle {
	QString source;
	std::optional<QSizeF> size;
};

using TextStyle = std::variant<ImageStyle,CharacterStyle>;

struct FormatContext {
	QString homeserver;
};

class ITextEntityFormatter {
public:
	virtual ~ITextEntityFormatter() { };
	virtual QRegularExpression matches() const = 0;
	virtual TextStyle styleFor(const QString& snippet, const FormatContext& ctx) const = 0;
};

class TextFormatter : public QObject {
	Q_OBJECT

public:
	explicit TextFormatter(QTextDocument* parent, const QString& homeserver, QObject* field);
	~TextFormatter();

	static QMap<QObject*,TextFormatter*> s_instances;

	void registerFormatter(ITextEntityFormatter* formatter);
	void removeTextFormatter(ITextEntityFormatter* formatter);
	QString plaintext(int from, int to);
	QString plaintext();

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

private:
	class Private;
	Private* p;

	void handleTextChanged(int position, int charsRemoved, int charsAdded);
};
