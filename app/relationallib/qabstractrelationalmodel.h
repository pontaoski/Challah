#pragma once

#include <QHash>
#include <QObject>
#include <QVariant>

class ChallahAbstractRelationalModel : public QObject
{

    Q_OBJECT

public:
    explicit ChallahAbstractRelationalModel(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(this, &ChallahAbstractRelationalModel::keyAdded, this, [this](const QVariant& key) {
            Q_EMIT(keyDataChanged(key, {}));
        });
        connect(this, &ChallahAbstractRelationalModel::keyRemoved, this, [this](const QVariant& key) {
            Q_EMIT(keyDataChanged(key, {}));
        });
    }
    virtual ~ChallahAbstractRelationalModel() {};

    Q_INVOKABLE virtual QVariant data(const QVariant& key, int role = Qt::DisplayRole) = 0;

    // check whether or not 'key' is a valid key
    Q_INVOKABLE virtual bool checkKey(const QVariant& key) = 0;

    Q_INVOKABLE virtual bool canFetchKey(const QVariant& key)
    {
        return false;
    }
    Q_INVOKABLE virtual void fetchKey(const QVariant& key)
    {
        Q_UNUSED(key)
    }

    Q_INVOKABLE virtual QHash<int, QByteArray> roleNames()
    {
        return {
            { Qt::DisplayRole, "display" },
            { Qt::DecorationRole, "decoration" },
            { Qt::EditRole, "edit" },
            { Qt::ToolTipRole, "toolTip" },
            { Qt::StatusTipRole, "statusTip" },
            { Qt::WhatsThisRole, "whatsThis" },
        };
    }

Q_SIGNALS:
    // automatically emit keyDataChanged
    void keyAdded(const QVariant& key);
    void keyRemoved(const QVariant& key);

    void keyDataChanged(const QVariant& key, const QVector<int>& roles);
};
