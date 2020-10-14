#pragma once

#include <QQuickItem>
#include <QVariantAnimation>

class OverlappingPanels : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem* leftPanel READ rightPanel WRITE setRightPanel NOTIFY rightPanelChanged REQUIRED)
	Q_PROPERTY(QQuickItem* rightPanel READ leftPanel WRITE setLeftPanel NOTIFY leftPanelChanged REQUIRED)
	Q_PROPERTY(QQuickItem* centerPanel READ centerPanel WRITE setCenterPanel NOTIFY centerPanelChanged REQUIRED)

	void handlePositionChange();
	void goToState(bool fling = false);
	qreal stateOffset() const;

public:
	OverlappingPanels(QQuickItem* parent = nullptr);

	enum EventKind {
		Down,
		Up,
		Move
	};

	enum State {
		Left,
		Center,
		Right
	};

	void mousePressEvent(QMouseEvent *event) override { mouseEvent(Down, event) ? event->accept() : event->ignore(); };
	void mouseReleaseEvent(QMouseEvent *event) override { mouseEvent(Up, event) ? event->accept() : event->ignore(); };
	void mouseMoveEvent(QMouseEvent *event) override { mouseEvent(Move, event) ? event->accept() : event->ignore(); };
	bool mouseEvent(EventKind kind, QMouseEvent* event);

	QVariantAnimation* m_animation;
	State m_state = Center;
	QPoint m_previousPoint = QPoint(0, 0);
	QPointF m_translation;

	QList<QMetaObject::Connection> m_centerPanelConnections;
	QQuickItem* m_centerPanel = nullptr;
	QQuickItem* centerPanel() const;
	void setCenterPanel(QQuickItem* item);
	Q_SIGNAL void centerPanelChanged();

	QList<QMetaObject::Connection> m_rightPanelConnections;
	QQuickItem* m_rightPanel = nullptr;
	QQuickItem* rightPanel() const;
	void setRightPanel(QQuickItem* item);
	Q_SIGNAL void rightPanelChanged();

	QList<QMetaObject::Connection> m_leftPanelConnections;
	QQuickItem* m_leftPanel = nullptr;
	QQuickItem* leftPanel() const;
	void setLeftPanel(QQuickItem* item);
	Q_SIGNAL void leftPanelChanged();
};
