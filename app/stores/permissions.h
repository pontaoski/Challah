#pragma once

#include <QAbstractListModel>

#include "state.h"

class PermissionsModel : public QAbstractListModel
{
	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	QString host;

	Q_PROPERTY(bool isDirty MEMBER isDirty NOTIFY isDirtyChanged)
	Q_PROPERTY(bool failure MEMBER failure NOTIFY failureChanged)

	Q_SIGNAL void isDirtyChanged();
	bool isDirty = false;

	Q_SIGNAL void failureChanged();
	bool failure = false;

public:
	PermissionsModel(QString host, quint64 guildID, quint64 roleID, State* state);
	~PermissionsModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() const override;

	Q_INVOKABLE void addPermission(const QString& node, bool allow);
	Q_INVOKABLE void deletePermission(int index);

	Q_INVOKABLE Croutons::FutureBase save();
};
