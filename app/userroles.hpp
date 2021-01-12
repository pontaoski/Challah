#pragma once

#include <QAbstractListModel>
#include <QSharedPointer>

#define invokable Q_INVOKABLE

class UserRolesModel : public QAbstractListModel
{
	Q_OBJECT

	struct Private;
	QSharedPointer<Private> d;

	Q_PROPERTY(bool errored READ errored CONSTANT)
	bool errored();

	Q_PROPERTY(bool editable READ editable CONSTANT)
	bool editable();

	Q_PROPERTY(QVariantList guildRoles READ guildRoles CONSTANT)
	QVariantList guildRoles();

public:
	UserRolesModel(quint64 userID, quint64 guildID, const QString& homeserver, QObject* parent);

	int rowCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& idx, int role) const override;
	QHash<int,QByteArray> roleNames() const override;

	invokable void remove(const QModelIndex& idx);
	invokable void add(const QString& role);
};
