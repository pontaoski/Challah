#pragma once

#include <QAbstractListModel>

class Client;

class PermissionsModel : public QAbstractListModel
{
	Q_OBJECT

	QString homeserver;
	quint64 guildID;
	quint64 roleID;

	Client* client;

	struct Private;
	Private* d;

	Q_PROPERTY(bool isDirty MEMBER isDirty NOTIFY isDirtyChanged)

	Q_SIGNAL void isDirtyChanged();
	bool isDirty = false;

protected:
	void customEvent(QEvent *event) override;

public:
	PermissionsModel(QString homeserver, quint64 guildID, quint64 roleID);
	~PermissionsModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
	QHash<int,QByteArray> roleNames() const override;

	Q_INVOKABLE void addPermission(const QString& node, bool allow);
	Q_INVOKABLE void deletePermission(int index);
	Q_INVOKABLE void save();
};
