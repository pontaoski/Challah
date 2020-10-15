#pragma once

#include <QQuickItem>
#include <QVariantAnimation>

// The horizontal center of the centre sheet must be in the outer 1/7ths of the area
// in order to transition state change.
const int THRESHOLD_FACTOR = 7;

// The hang factor determines how much of the centre sheet will be visible.
// The current hang factor is 10, which means the outermost 10% of a side will be
// shown.
const int HANG_FACTOR = 10;

const int PANEL_OPEN_MS = 250;
const int PANEL_CLOSE_MS = 200;

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

	template <typename T>
	bool handlePointerEvent(EventKind kind, T event)
	{
		if (m_expanded) {
			return false;
		}

		static bool down = false;
		static bool inThreshold = false;

		if (kind == EventKind::Down) {
			down = true;
			setPreviousPoint(event->pos());

			if (m_state == State::Left) {
				const auto leftThreshold = (width() / THRESHOLD_FACTOR);
				if (event->pos().x() <= leftThreshold) {
					inThreshold = true;
				} else {
					inThreshold = false;
				}
			} else if (m_state == State::Right) {
				const auto rightThreshold = (width() - (width() / THRESHOLD_FACTOR));
				if (event->pos().x() >= rightThreshold) {
					inThreshold = true;
				} else {
					inThreshold = false;
				}
			} else {
				inThreshold = false;
			}

			return true;
		} else if (kind == EventKind::Up) {
			down = false;

			if (inThreshold) {
				m_state = State::Center;
				m_translation = QPointF();
				goToState(false);

				return true;
			}

			// Adjusted is the horizontal center of the centre panel
			const auto adjusted = m_centerPanel->x() + (width() / 2);

			const auto leftThreshold = (width() / THRESHOLD_FACTOR);
			const auto rightThreshold = (width() - (width() / THRESHOLD_FACTOR));

			// If adjusted is to the left of the left threshold...
			if (adjusted <= leftThreshold) {
				m_state = State::Left;
			}
			// If adjusted is to the right of the right threshold...
			else if (adjusted >= rightThreshold) {
				m_state = State::Right;
			}
			// If it's neither...
			else {
				m_state = State::Center;
			}

			m_translation = QPointF();

			goToState(true);

			return true;
		} else if (kind == EventKind::Move && down) {
			auto delta = m_previousPoint - event->pos();
			setPreviousPoint(event->pos());

			inThreshold = false;

			m_translation += delta;
			handlePositionChange();

			return true;
		}

		return false;
	}

	void setPreviousPoint(const QPoint& point) {
		m_previousPoint = point;
	}
	void setPreviousPoint(const QPointF& point) {
		m_previousPoint = point.toPoint();
	}

	void mousePressEvent(QMouseEvent *event) override { handlePointerEvent(Down, event) ? event->accept() : event->ignore(); };
	void mouseReleaseEvent(QMouseEvent *event) override { handlePointerEvent(Up, event) ? event->accept() : event->ignore(); };
	void mouseMoveEvent(QMouseEvent *event) override { handlePointerEvent(Move, event) ? event->accept() : event->ignore(); };
	void touchEvent(QTouchEvent *event) override {
		if (m_expanded) {
			event->ignore();
			return;
		}
		if (event->touchPoints().length() == 0) {
			event->ignore();
			return;
		}
		auto point = event->touchPoints()[0];
		EventKind kind = (EventKind)-1;
		switch (point.state()) {
		case Qt::TouchPointPressed:
			kind = Down; break;
		case Qt::TouchPointMoved:
			kind = Move; break;
		case Qt::TouchPointReleased:
			kind = Up; break;
		}
		if (kind == -1) {
			event->ignore();
			return;
		}
		handlePointerEvent(kind, &point) ? event->accept() : event->ignore();
	}

	QVariantAnimation* m_animation;
	QVariantAnimation* m_expansionAnimation;
	qreal m_expansionFromWidth;
	qreal m_expansionFromX;
	State m_state = Center;
	QPoint m_previousPoint = QPoint(0, 0);
	QPointF m_translation;
	bool m_expanded = false;

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
