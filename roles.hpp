#pragma once

#include <QAbstractListModel>

class Client;

class RolesModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeServer;
	quint64 guildID;

	Client* client;

	enum Roles {
		NameRole = Qt::UserRole,
		ColorRole,
		Permissions,
	};

public:
	RolesModel(QString homeServer, quint64 guildID);
	~RolesModel();

	Q_INVOKABLE void moveRoleFromTo(int from, int to);
	Q_INVOKABLE bool createRole(const QString& name, const QColor& colour);
	Q_INVOKABLE QVariant everyonePermissions() const;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() const override;

	struct Private;
	Private* d;
};
