#pragma once

#include <QAbstractListModel>

#include "state.h"

class PermissionsModel : public QAbstractListModel
{
	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	SDK::Client* c;

	Q_PROPERTY(bool isDirty MEMBER isDirty NOTIFY isDirtyChanged)

	Q_SIGNAL void isDirtyChanged();
	bool isDirty = false;

public:
	PermissionsModel(SDK::Client* client, quint64 guildID, quint64 roleID, State* state);
	~PermissionsModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() const override;

	Q_INVOKABLE void addPermission(const QString& node, bool allow);
	Q_INVOKABLE void deletePermission(int index);

	Q_INVOKABLE FutureBase save();
};
