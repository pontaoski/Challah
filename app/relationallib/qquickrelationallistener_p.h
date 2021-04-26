#pragma once

#include "qquickrelationallistener.h"

class ChallahQmlRelationalListenerPrivate
{
    Q_DECLARE_PUBLIC(ChallahQmlRelationalListener)
    ChallahQmlRelationalListener* q_ptr;

public:
    ChallahQmlRelationalListenerPrivate(ChallahQmlRelationalListener* ptr)
        : q_ptr(ptr)
    {
    }

    QPointer<ChallahAbstractRelationalModel> relationalModel = nullptr;
    QPointer<QQmlComponent> shape = nullptr;
    QVariant key = QVariant();
    QObject* dataObject = nullptr;
    bool complete = false;
    bool enabled = true;
};
