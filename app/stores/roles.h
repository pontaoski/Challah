#pragma once

#include <QAbstractListModel>
#include <QColor>

#include "state.h"

class RolesModel : public QAbstractListModel
{
	Q_OBJECT

	struct Private;
	QScopedPointer<Private> d;

	State* s;
	QString host;

	Q_PROPERTY(bool working READ working NOTIFY workingChanged)

public:
	RolesModel(QString host, quint64 guildID, State* state);
	~RolesModel();

	bool working() const;
	Q_SIGNAL void workingChanged();

	Q_INVOKABLE Croutons::FutureBase moveRoleFromTo(int from, int to);
	Q_INVOKABLE Croutons::FutureBase moveRole(const QString& id, int to);
	Q_INVOKABLE Croutons::FutureBase createRole(QString name, QColor colour);
	Q_INVOKABLE QVariant everyonePermissions() const;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() const override;

};
