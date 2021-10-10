#include <QQmlEngine>
#include <QJsonValue>
#include <QJsonArray>
#include <QJSValue>

#include <cstring>

#include "qquickrelationallistener_p.h"

ChallahQmlRelationalListener::ChallahQmlRelationalListener(QObject* parent)
    : QObject(parent)
    , QQmlParserStatus()
    , d_ptr(new ChallahQmlRelationalListenerPrivate(this))
{
}

ChallahQmlRelationalListener::~ChallahQmlRelationalListener()
{

}

auto normaliseVariant(const QVariant& variant) -> QVariant
{
    if (variant.canConvert<QJSValue>()) {
        return variant.value<QJSValue>().toVariant();
    } else if (variant.canConvert<QJsonArray>()) {
        return variant.value<QJsonArray>().toVariantList();
    } else if (variant.canConvert<QJsonValue>()) {
        return variant.value<QJsonValue>().toVariant();
    } else {
        return variant;
    }
}

void ChallahQmlRelationalListener::newRelationalModel(ChallahAbstractRelationalModel* model)
{
    if (d_ptr->relationalModel) {
        disconnect(d_ptr->relationalModel, &ChallahAbstractRelationalModel::keyDataChanged, this, nullptr);
    }

    d_ptr->relationalModel = model;

    if (!d_ptr->relationalModel) {
        return;
    }

    connect(d_ptr->relationalModel, &ChallahAbstractRelationalModel::keyDataChanged, this, [this](const QVariant& key, const QVector<int>& roles) {
        if (d_ptr->key != normaliseVariant(key)) {
            return;
        }

        applyChanged(roles);
    });
}

void ChallahQmlRelationalListener::componentComplete()
{
    Q_ASSERT(!d_ptr->shape.isNull());
    Q_ASSERT(d_ptr->shape->isReady());

    checkKey();
    applyChanged({});
    d_ptr->complete = true;
}

void ChallahQmlRelationalListener::checkKey()
{
    if (!d_ptr->relationalModel) return;
    if (d_ptr->key == QVariant()) return;
    if (!d_ptr->enabled) return;

    if (!d_ptr->relationalModel->checkKey(d_ptr->key)) {
        if (d_ptr->relationalModel->canFetchKey(d_ptr->key)) {
            d_ptr->relationalModel->fetchKey(d_ptr->key);
        }
    } else {
        applyChanged({});
    }
}

bool ChallahQmlRelationalListener::enabled() const
{
    return d_ptr->enabled;
}

void ChallahQmlRelationalListener::setEnabled(bool enabled)
{
    if (d_ptr->enabled == enabled) {
        return;
    }

    d_ptr->enabled = enabled;
    Q_EMIT enabledChanged();
    checkKey();
}

void ChallahQmlRelationalListener::resetEnabled()
{
    setEnabled(false);
}

void ChallahQmlRelationalListener::applyChanged(const QVector<int>& roles)
{
    if (!d_ptr->relationalModel || !d_ptr->enabled) return;

    const auto roleNames = d_ptr->relationalModel->roleNames();
    QHash<QByteArray,int> invertedRoleNames;
    for (auto it : roleNames.keys()) {
        invertedRoleNames[roleNames[it]] = it;
    }

    QVariantMap props;

    auto on = d_ptr->dataObject;
    auto fromScratch = on == nullptr;
    if (fromScratch) {
        if (d_ptr->shape->status() == QQmlComponent::Error) {
            qFatal("%s", d_ptr->shape->errorString().toStdString().c_str());
        }
        on = d_ptr->shape->beginCreate(qmlContext(this));
    }
    auto mo = on->metaObject();

    for (int i = mo->propertyOffset(); i < mo->propertyCount(); i++) {
        QByteArray propName = mo->property(i).name();

        if (!invertedRoleNames.contains(propName)) {
            qWarning() << "Model doesn't contain" << propName.toStdString().c_str();
            continue;
        }

        auto role = invertedRoleNames[propName];
        if (!roles.contains(role) && !roles.isEmpty()) {
            continue;
        }

        if (!fromScratch) {
            mo->property(i).write(d_ptr->dataObject, d_ptr->relationalModel->data(d_ptr->key, role));
        } else {
            props[propName] = d_ptr->relationalModel->data(d_ptr->key, role);
        }
    }

    if (fromScratch) {
        d_ptr->shape->setInitialProperties(on, props);
        d_ptr->shape->completeCreate();
        d_ptr->dataObject = on;
        Q_EMIT dataChanged();
    }
}

ChallahAbstractRelationalModel* ChallahQmlRelationalListener::model() const
{
    return d_ptr->relationalModel;
}

void ChallahQmlRelationalListener::setModel(ChallahAbstractRelationalModel* setModel)
{
    if (setModel == d_ptr->relationalModel) {
        return;
    }

    newRelationalModel(setModel);
    Q_EMIT modelChanged();
    checkKey();
}

void ChallahQmlRelationalListener::resetModel()
{
    setModel(nullptr);
}

QVariant ChallahQmlRelationalListener::key() const
{
    return d_ptr->key;
}

void ChallahQmlRelationalListener::setKey(const QVariant& key)
{
    if (key == d_ptr->key) {
        return;
    }

    d_ptr->key = normaliseVariant(key);
    Q_EMIT keyChanged();
    if (d_ptr->complete) {
        checkKey();
    }
}

void ChallahQmlRelationalListener::resetKey()
{
    setKey(QVariant());
}

QQmlComponent* ChallahQmlRelationalListener::shape() const
{
    return d_ptr->shape;
}

void ChallahQmlRelationalListener::setShape(QQmlComponent* shape)
{
    if (shape == d_ptr->shape) {
        return;
    }

    d_ptr->shape = shape;
    Q_EMIT shapeChanged();
}

QObject* ChallahQmlRelationalListener::data() const
{
    return d_ptr->dataObject;
}
