#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <QQmlComponent>

#include "qabstractrelationalmodel.h"

class ChallahQmlRelationalListenerPrivate;

class ChallahQmlRelationalListener : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(ChallahAbstractRelationalModel* model READ model WRITE setModel RESET resetModel NOTIFY modelChanged)
    Q_PROPERTY(QVariant key READ key WRITE setKey RESET resetKey NOTIFY keyChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled RESET resetEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QQmlComponent* shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(QObject* data READ data NOTIFY dataChanged)

public:
    explicit ChallahQmlRelationalListener(QObject* parent = nullptr);
    ~ChallahQmlRelationalListener();

    void classBegin() override {}
    void componentComplete() override;

    ChallahAbstractRelationalModel* model() const;
    void setModel(ChallahAbstractRelationalModel* setModel);
    void resetModel();
    Q_SIGNAL void modelChanged();

    QVariant key() const;
    void setKey(const QVariant& key);
    void resetKey();
    Q_SIGNAL void keyChanged();

    bool enabled() const;
    void setEnabled(bool enabled);
    void resetEnabled();
    Q_SIGNAL void enabledChanged();

    QQmlComponent* shape() const;
    void setShape(QQmlComponent* shape);
    Q_SIGNAL void shapeChanged();

    QObject* data() const;
    Q_SIGNAL void dataChanged();

private:
    Q_DECLARE_PRIVATE(ChallahQmlRelationalListener)
    std::unique_ptr<ChallahQmlRelationalListenerPrivate> d_ptr;

    void applyChanged(const QVector<int>& roles);
    void checkKey();

};
