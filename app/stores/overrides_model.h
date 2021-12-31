#pragma once

#include <QAbstractListModel>

class State;

struct OverrideData
{
	Q_GADGET

	Q_PROPERTY(QString name MEMBER name)
	Q_PROPERTY(QString avatar MEMBER avatar)

public:
	QString name;
	QString avatar;

	OverrideData();
	OverrideData(const QString& name, const QString& avatar)
		: name(name)
		, avatar(avatar)
	{

	}
	~OverrideData();
};
Q_DECLARE_METATYPE(OverrideData)

class OverridesModel : public QAbstractListModel
{

	Q_OBJECT

	struct Private;
	std::unique_ptr<Private> d;

	State* s;

	Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)

public:

	explicit OverridesModel(QObject* parent, State* state);
	~OverridesModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() const override;

	Q_INVOKABLE void addOverride(const QString& name, const QString& avatar, const QString& before, const QString& after);
	Q_INVOKABLE void save();

	bool dirty() const;
	Q_SIGNAL void dirtyChanged();

	int count() const;
	Q_SIGNAL void countChanged();

	friend class MessagesModel;

};
