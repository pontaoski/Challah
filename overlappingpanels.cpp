#include "overlappingpanels.hpp"

const QEasingCurve FLING_CURVE = [](){
	QEasingCurve curve;
	curve.setType(QEasingCurve::BezierSpline);
	curve.addCubicBezierSegment(QPointF(0.0, 0.0), QPointF(0.2, 1.0), QPointF(1.0, 1.0));
	return curve;
}();

const QEasingCurve TAP_CURVE = [](){
	QEasingCurve curve;
	curve.setType(QEasingCurve::BezierSpline);
	curve.addCubicBezierSegment(QPointF(0.4, 0.0), QPointF(0.2, 1.0), QPointF(1.0, 1.0));
	return curve;
}();

auto linearlyInterpolateDouble(double one, double two, double factor) {
	return one + (two - one) * factor;
};


OverlappingPanels::OverlappingPanels(QQuickItem* parent) : QQuickItem(parent)
{
	m_animation = new QVariantAnimation(this);
	connect(this, &QQuickItem::widthChanged, [=]() { handlePositionChange(); });
	connect(this, &QQuickItem::heightChanged, [=]() { handlePositionChange(); });

	m_expansionAnimation = new QVariantAnimation(this);

	m_expansionAnimation->setStartValue(0.0);
	m_expansionAnimation->setEndValue(1.0);
	m_expansionAnimation->setEasingCurve(TAP_CURVE);

	connect(m_expansionAnimation, &QVariantAnimation::valueChanged, [=](const QVariant& value) {
		auto progress = value.toReal();

		if (m_expanded) {
			auto targetWidth = width() - m_leftPanel->implicitWidth() - m_rightPanel->implicitWidth();

			auto currentWidth = linearlyInterpolateDouble(m_expansionFromWidth, targetWidth, progress);
			auto currentX = linearlyInterpolateDouble(m_expansionFromX, m_leftPanel->implicitWidth(), progress);

			m_centerPanel->blockSignals(true);
			m_centerPanel->setX(currentX);
			m_centerPanel->setWidth(currentWidth);
			m_centerPanel->blockSignals(false);
		} else {
			auto targetWidth = width();

			auto currentWidth = linearlyInterpolateDouble(m_expansionFromWidth, targetWidth, progress);
			auto currentX = linearlyInterpolateDouble(m_expansionFromX, 0, progress);

			m_centerPanel->blockSignals(true);
			m_centerPanel->setX(currentX);
			m_centerPanel->setWidth(currentWidth);
			m_centerPanel->blockSignals(false);
		}
	});
	connect(m_expansionAnimation, &QVariantAnimation::finished, [=]() {
		if (!m_expanded) {
			m_leftPanel->blockSignals(true);
			m_rightPanel->blockSignals(true);

			m_leftPanel->setVisible(false);
			m_rightPanel->setVisible(false);

			m_rightPanel->setPosition(QPointF(0, 0));
			m_rightPanel->setWidth(width() - (width() / HANG_FACTOR));

			m_leftPanel->setPosition(QPointF(width() - m_leftPanel->width(), 0));
			m_leftPanel->setWidth(width() - (width() / HANG_FACTOR));

			m_leftPanel->blockSignals(false);
			m_rightPanel->blockSignals(false);
		}
	});

	connect(m_animation, &QVariantAnimation::valueChanged, [=](const QVariant& value) {
		if (m_animation->state() != QAbstractAnimation::Stopped && m_expansionAnimation->state() != QAbstractAnimation::Running) {
			m_centerPanel->setX(value.toReal());
		}
	});
	connect(m_animation, &QVariantAnimation::finished, [=]() {
		if (m_state != State::Center) {
			m_centerPanel->setOpacity(0.7);
		} else {
			m_centerPanel->setOpacity(1.0);
		}
		handlePositionChange();
	});

	setAcceptedMouseButtons(Qt::LeftButton);
	setAcceptTouchEvents(true);
}

void OverlappingPanels::handlePositionChange()
{
	if (m_leftPanel == nullptr && m_centerPanel == nullptr && m_rightPanel == nullptr) {
		return;
	}

	if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
		return;
	}

	if (m_leftPanel->implicitWidth() + m_centerPanel->implicitWidth() + m_rightPanel->implicitWidth() <= width()) {
		if (!m_expanded) {
			m_expanded = true;
			m_expansionFromWidth = m_centerPanel->width();
			m_expansionFromX = m_centerPanel->x();

			m_centerPanel->setOpacity(1.0);
			m_expansionAnimation->start();

			m_leftPanel->blockSignals(true);
			m_rightPanel->blockSignals(true);

			m_leftPanel->setWidth(m_leftPanel->implicitWidth());
			m_rightPanel->setWidth(m_rightPanel->implicitWidth());

			m_leftPanel->setPosition(QPointF(0, 0));
			m_rightPanel->setPosition(QPointF(m_leftPanel->width() + (width() - m_leftPanel->implicitWidth() - m_rightPanel->implicitWidth()), 0));

			m_leftPanel->setVisible(true);
			m_rightPanel->setVisible(true);

			m_leftPanel->blockSignals(false);
			m_rightPanel->blockSignals(false);
			return;
		}

		m_leftPanel->setVisible(true);
		m_rightPanel->setVisible(true);

		m_leftPanel->blockSignals(true);
		m_rightPanel->blockSignals(true);
		m_centerPanel->blockSignals(true);

		m_leftPanel->setWidth(m_leftPanel->implicitWidth());
		m_rightPanel->setWidth(m_rightPanel->implicitWidth());
		m_centerPanel->setWidth(width() - m_leftPanel->implicitWidth() - m_rightPanel->implicitWidth());

		m_leftPanel->setPosition(QPointF(0, 0));
		m_centerPanel->setPosition(QPointF(m_leftPanel->width(), 0));
		m_rightPanel->setPosition(QPointF(m_leftPanel->width() + m_centerPanel->width(), 0));

		m_leftPanel->blockSignals(false);
		m_rightPanel->blockSignals(false);
		m_centerPanel->blockSignals(false);

		return;
	} else if (m_expanded) {
		m_expanded = false;
		m_state = State::Center;

		m_leftPanel->blockSignals(true);
		m_rightPanel->blockSignals(true);

		m_leftPanel->setWidth(width() / 2);
		m_rightPanel->setWidth(width() / 2);

		m_leftPanel->setX(0);
		m_rightPanel->setX(width() - m_rightPanel->width());

		m_leftPanel->blockSignals(false);
		m_rightPanel->blockSignals(false);

		m_expansionFromWidth = m_centerPanel->width();
		m_expansionFromX = m_centerPanel->x();

		m_expansionAnimation->start();
		return;
	}

	auto clamped = stateOffset() + (-m_translation.x());
	if (m_state == State::Left && clamped <= stateOffset()) {
		clamped = stateOffset();
		m_translation.setX(0);
	} else if (m_state == State::Right && clamped >= stateOffset()) {
		clamped = stateOffset();
		m_translation.setX(0);
	}
	m_centerPanel->setX(clamped);

	auto containerHorizontalCenter = (width() / 2);
	auto itemHorizontalCenter = clamped + containerHorizontalCenter;
	if (itemHorizontalCenter < containerHorizontalCenter) {
		m_leftPanel->setVisible(true);
		m_rightPanel->setVisible(false);
	} else if (itemHorizontalCenter > containerHorizontalCenter) {
		m_rightPanel->setVisible(true);
		m_leftPanel->setVisible(false);
	} else {
		m_leftPanel->setVisible(false);
		m_rightPanel->setVisible(false);
	}
}

void OverlappingPanels::goToState(bool fling)
{
	if (fling) {
		m_animation->setEasingCurve(FLING_CURVE);
	} else {
		m_animation->setEasingCurve(TAP_CURVE);
	}

	m_animation->setStartValue(m_centerPanel->x());
	m_animation->setEndValue(stateOffset());

	if (m_state == State::Center) {
		m_animation->setDuration(PANEL_CLOSE_MS);
	} else {
		m_animation->setDuration(PANEL_OPEN_MS);
	}

	if (m_centerPanel->x() != stateOffset()) {
		m_animation->stop();
		m_animation->start();
	}
}

qreal OverlappingPanels::stateOffset() const
{
	switch (m_state) {
	case State::Center:
		return 0;
	case State::Right:
		return width() - (width() / HANG_FACTOR);
	case State::Left:
		return -width() + (width() / HANG_FACTOR);
	}

	return 0;
}

QQuickItem* OverlappingPanels::centerPanel() const
{
	return m_centerPanel;
}

void OverlappingPanels::setCenterPanel(QQuickItem* item)
{
	Q_ASSERT(item != nullptr);

	if (item != m_centerPanel) {
		m_centerPanel = item;

		for (auto sig : m_centerPanelConnections) {
			QObject::disconnect(sig);
		}
		m_centerPanelConnections.clear();

		item->setParentItem(this);
		item->setPosition(QPointF(0, 0));
		item->setWidth(width());
		item->setHeight(height());

		item->stackAfter(m_leftPanel);
		item->stackAfter(m_rightPanel);

		m_centerPanelConnections << connect(this, &QQuickItem::widthChanged, [=]() { item->setWidth(width()); });
		m_centerPanelConnections << connect(this, &QQuickItem::heightChanged, [=]() { item->setHeight(height()); });
		m_centerPanelConnections << connect(item, &QQuickItem::widthChanged, [=]() { item->setWidth(width()); handlePositionChange(); });
		m_centerPanelConnections << connect(item, &QQuickItem::heightChanged, [=]() { item->setHeight(height()); handlePositionChange(); });

		Q_EMIT centerPanelChanged();
	}
}

QQuickItem* OverlappingPanels::rightPanel() const
{
	return m_rightPanel;
}

void OverlappingPanels::setRightPanel(QQuickItem* item)
{
	Q_ASSERT(item != nullptr);

	if (item != m_rightPanel) {
		m_rightPanel = item;

		for (auto sig : m_rightPanelConnections) {
			QObject::disconnect(sig);
		}
		m_rightPanelConnections.clear();

		item->setParentItem(this);
		item->setPosition(QPointF(0, 0));
		item->setWidth(width() - (width() / HANG_FACTOR));
		item->setHeight(height());

		qDebug() << width() << (width() - (width() / HANG_FACTOR));

		item->stackBefore(m_centerPanel);

		m_rightPanelConnections << connect(this, &QQuickItem::widthChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setWidth(width() - (width() / HANG_FACTOR));
		});
		m_rightPanelConnections << connect(this, &QQuickItem::heightChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setHeight(height());
		});
		m_rightPanelConnections << connect(item, &QQuickItem::widthChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setWidth(width() - (width() / HANG_FACTOR)); handlePositionChange();
		});
		m_rightPanelConnections << connect(item, &QQuickItem::heightChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setHeight(height()); handlePositionChange();
		});

		Q_EMIT rightPanelChanged();
	}
}

QQuickItem* OverlappingPanels::leftPanel() const
{
	return m_leftPanel;
}

void OverlappingPanels::setLeftPanel(QQuickItem* item)
{
	Q_ASSERT(item != nullptr);

	Q_ASSERT(item != nullptr);

	if (item != m_leftPanel) {
		m_leftPanel = item;

		for (auto sig : m_leftPanelConnections) {
			QObject::disconnect(sig);
		}
		m_leftPanelConnections.clear();

		item->setParentItem(this);
		item->setPosition(QPointF(width() - item->width(), 0));
		item->setWidth(width() - (width() / HANG_FACTOR));
		item->setHeight(height());

		item->stackBefore(m_centerPanel);

		m_leftPanelConnections << connect(this, &QQuickItem::widthChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setWidth(width() - (width() / HANG_FACTOR));
			item->setPosition(QPointF(width() - item->width(), 0));
		});
		m_leftPanelConnections << connect(this, &QQuickItem::heightChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setHeight(height());
		});
		m_leftPanelConnections << connect(item, &QQuickItem::widthChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setWidth(width() - (width() / HANG_FACTOR));
			item->setPosition(QPointF(width() - item->width(), 0));
			handlePositionChange();
		});
		m_leftPanelConnections << connect(item, &QQuickItem::heightChanged, [=]() {
			if (m_expansionAnimation->state() == QAbstractAnimation::Running) {
				return;
			}
			if (m_expanded) {
				handlePositionChange();
				return;
			}
			item->setHeight(height()); handlePositionChange();
		});

		Q_EMIT leftPanelChanged();
	}
}
