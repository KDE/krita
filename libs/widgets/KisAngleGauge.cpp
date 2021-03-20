/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>
#include <QMouseEvent>
#include <cmath>

#include "KisAngleGauge.h"

struct KisAngleGauge::Private
{
    static constexpr qreal minimumSnapDistance{40.0};
    qreal angle;
    qreal snapAngle;
    qreal resetAngle;
    IncreasingDirection increasingDirection;
    bool isPressed;
    bool isMouseHover;
};

KisAngleGauge::KisAngleGauge(QWidget* parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->angle = 0.0;
    m_d->snapAngle = 15.0;
    m_d->resetAngle = 0.0;
    m_d->increasingDirection = IncreasingDirection_CounterClockwise;
    m_d->isPressed = false;
    m_d->isMouseHover = false;
    
    setFocusPolicy(Qt::WheelFocus);
}

KisAngleGauge::~KisAngleGauge()
{}

qreal KisAngleGauge::angle() const
{
    return m_d->angle;
}

qreal KisAngleGauge::snapAngle() const
{
    return m_d->snapAngle;
}

qreal KisAngleGauge::resetAngle() const
{
    return m_d->resetAngle;
}

KisAngleGauge::IncreasingDirection KisAngleGauge::increasingDirection() const
{
    return m_d->increasingDirection;
}

void KisAngleGauge::setAngle(qreal newAngle)
{
    if (qFuzzyCompare(newAngle, m_d->angle)) {
        return;
    }

    m_d->angle = newAngle;
    update();
    emit angleChanged(newAngle);
}

void KisAngleGauge::setSnapAngle(qreal newSnapAngle)
{
    m_d->snapAngle = newSnapAngle;
}

void KisAngleGauge::setResetAngle(qreal newResetAngle)
{
    m_d->resetAngle = newResetAngle;
}

void KisAngleGauge::setIncreasingDirection(IncreasingDirection newIncreasingDirection)
{
    m_d->increasingDirection = newIncreasingDirection;
    update();
}

void KisAngleGauge::reset()
{
    setAngle(resetAngle());
}

void KisAngleGauge::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal minSide = std::min(center.x(), center.y());
    const qreal radius = minSide * 0.9;
    const qreal lineMarkerRadius = minSide * 0.1;
    const qreal angleInRadians = m_d->angle * M_PI / 180.0;
    const QPointF d(
        center.x() + std::cos(angleInRadians) * radius,
        m_d->increasingDirection == IncreasingDirection_CounterClockwise
        ? center.y() - std::sin(angleInRadians) * radius
        : center.y() + std::sin(angleInRadians) * radius
    );

    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor backgroundColor, circleColor, axesColor, angleLineColor, angleLineMarkerColor;
    if (palette().color(QPalette::Window).lightness() < 128) {
        circleColor = palette().color(QPalette::Light);
        axesColor = palette().color(QPalette::Light);
        axesColor.setAlpha(200);
        if (isEnabled()) {
            backgroundColor = palette().color(QPalette::Dark);
            angleLineColor = QColor(255, 255, 255, 128);
            angleLineMarkerColor = QColor(255, 255, 255, 200);
        } else {
            backgroundColor = palette().color(QPalette::Window);
            angleLineColor = palette().color(QPalette::Light);
            angleLineMarkerColor = palette().color(QPalette::Light);
        }
    } else {
        circleColor = palette().color(QPalette::Dark);
        axesColor = palette().color(QPalette::Dark);
        axesColor.setAlpha(200);
        if (isEnabled()) {
            backgroundColor = palette().color(QPalette::Light);
            angleLineColor = QColor(0, 0, 0, 128);
            angleLineMarkerColor = QColor(0, 0, 0, 200);
        } else {
            backgroundColor = palette().color(QPalette::Window);
            angleLineColor = palette().color(QPalette::Dark);
            angleLineMarkerColor = palette().color(QPalette::Dark);
        }
    }

    // Background
    painter.setPen(Qt::transparent);
    painter.setBrush(backgroundColor);
    painter.drawEllipse(center, radius, radius);

    // Axes lines
    painter.setPen(QPen(axesColor, 1.0, Qt::DotLine));
    painter.drawLine(center.x(), center.y() - radius + 1.0, center.x(), center.y() + radius - 1.0);
    painter.drawLine(center.x() - radius + 1.0, center.y(), center.x() + radius - 1.0, center.y());

    // Outer circle
    if (this->hasFocus()) {
        painter.setPen(QPen(palette().color(QPalette::Highlight), 2.0));
    } else {
        if (m_d->isMouseHover) {
            painter.setPen(QPen(palette().color(QPalette::Highlight), 1.0));
        } else {
            painter.setPen(QPen(circleColor, 1.0));
        }
    }
    painter.setBrush(Qt::transparent);
    painter.drawEllipse(center, radius, radius);

    // Angle line
    painter.setPen(QPen(angleLineColor, 1.0));
    painter.drawLine(center, d);

    // Inner line marker
    painter.setPen(Qt::transparent);
    painter.setBrush(angleLineMarkerColor);
    painter.drawEllipse(center, lineMarkerRadius, lineMarkerRadius);

    // Outer line marker
    painter.setBrush(angleLineMarkerColor);
    painter.drawEllipse(d, lineMarkerRadius, lineMarkerRadius);

    e->accept();
}

void KisAngleGauge::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }

    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal radius = std::min(center.x(), center.y());
    const qreal radiusSquared = radius * radius;
    const QPointF delta(e->x() - center.x(), e->y() - center.y());
    const qreal distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();

    if (distanceSquared > radiusSquared) {
        e->ignore();
        return;
    }

    qreal angle =
        std::atan2(
            m_d->increasingDirection == IncreasingDirection_CounterClockwise ? -delta.y() : delta.y(),
            delta.x()
        );
    
    if (e->modifiers() & Qt::ControlModifier)  {
        const qreal sa = m_d->snapAngle * M_PI / 180.0;
        angle = std::round(angle / sa) * sa;
    }

    setAngle(angle * 180.0 / M_PI);

    m_d->isPressed = true;
    
    e->accept();
}

void KisAngleGauge::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_d->isPressed) {
        m_d->isPressed = false;
        e->accept();
        return;
    }
    e->ignore();
}

void KisAngleGauge::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton) || !m_d->isPressed) {
        e->ignore();
        return;
    }

    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal radius = std::min(center.x(), center.y());
    const qreal radiusSquared = radius * radius;
    const QPointF delta(e->x() - center.x(), e->y() - center.y());
    const qreal distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();
    qreal angle =
        std::atan2(
            m_d->increasingDirection == IncreasingDirection_CounterClockwise ? -delta.y() : delta.y(),
            delta.x()
        );

    const qreal snapDistance = qMax(m_d->minimumSnapDistance * m_d->minimumSnapDistance, radiusSquared * 4.0);
    if ((e->modifiers() & Qt::ControlModifier) || distanceSquared < snapDistance) {
        const qreal sa = m_d->snapAngle * M_PI / 180.0;
        angle = std::round(angle / sa) * sa;
    }

    setAngle(angle * 180.0 / M_PI);
    
    e->accept();
}

void KisAngleGauge::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        reset();
        e->accept();
    } else {
        e->ignore();
    }
}

void KisAngleGauge::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() > 0) {
        if (e->modifiers() & Qt::ControlModifier) {
            setAngle(std::floor((m_d->angle + m_d->snapAngle) / m_d->snapAngle) * m_d->snapAngle);
        } else {
            setAngle(m_d->angle + 1.0);
        }
    } else if (e->angleDelta().y() < 0) {
        if (e->modifiers() & Qt::ControlModifier) {
            setAngle(std::ceil((m_d->angle - m_d->snapAngle) / m_d->snapAngle) * m_d->snapAngle);
        } else {
            setAngle(m_d->angle - 1.0);
        }
    }
    e->accept();
}

void KisAngleGauge::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Right) {
        if (e->modifiers() & Qt::ControlModifier) {
            setAngle(std::floor((m_d->angle + m_d->snapAngle) / m_d->snapAngle) * m_d->snapAngle);
        } else {
            setAngle(m_d->angle + 1.0);
        }
        e->accept();
    } else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Left) {
        if (e->modifiers() & Qt::ControlModifier) {
            setAngle(std::ceil((m_d->angle - m_d->snapAngle) / m_d->snapAngle) * m_d->snapAngle);
        } else {
            setAngle(m_d->angle - 1.0);
        }
        e->accept();
    } else {
        e->ignore();
    }
}

void KisAngleGauge::enterEvent(QEvent *e)
{
    m_d->isMouseHover = true;
    update();
    QWidget::enterEvent(e);
}

void KisAngleGauge::leaveEvent(QEvent *e)
{
    m_d->isMouseHover = false;
    update();
    QWidget::leaveEvent(e);
}
