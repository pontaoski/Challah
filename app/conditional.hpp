#pragma once

#include <QObject>
#include <QQmlComponent>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QtQml>

#define signal Q_SIGNAL void

class Conditional : public QObject, public QQmlParserStatus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)

	Q_PROPERTY(bool value MEMBER m_value NOTIFY valueChanged)
	Q_PROPERTY(QQuickItem* pred MEMBER m_pred NOTIFY predChanged)
	Q_PROPERTY(QQuickItem* succ MEMBER m_succ NOTIFY succChanged)
	Q_PROPERTY(QQmlComponent* ifTrue MEMBER m_ifTrue NOTIFY ifTrueChanged)
	Q_PROPERTY(QQmlComponent* ifFalse MEMBER m_ifFalse NOTIFY ifFalseChanged)

	bool m_previousVal = false;
	bool m_ifTrueDirty = false;
	bool m_ifFalseDirty = false;
	QQuickItem* m_inst = nullptr;

	void applyChanges();

public:
	void classBegin() override {}
	void componentComplete() override;

	bool m_value = false;
	signal valueChanged();

	QQmlComponent* m_ifTrue = nullptr;
	signal ifTrueChanged();

	QQmlComponent* m_ifFalse = nullptr;
	signal ifFalseChanged();

	QQuickItem* m_pred = nullptr;
	signal predChanged();

	QQuickItem* m_succ = nullptr;
	signal succChanged();
};
