/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_draggable_tool_button.h"

#include <QMouseEvent>

#include "kis_debug.h"


KisDraggableToolButton::KisDraggableToolButton(QWidget *parent)
    : QToolButton(parent),
      m_orientation(Qt::Horizontal)
{
}

KisDraggableToolButton::~KisDraggableToolButton()
{
}

void KisDraggableToolButton::beginDrag(const QPoint &pos)
{
    m_startPoint = pos;
    m_lastPosition = m_startPoint;
}

int KisDraggableToolButton::continueDrag(const QPoint &pos)
{
    QPoint diff = pos - m_startPoint;

    int value = 0;

    qreal tanx = diff.x() != 0 ? qAbs(qreal(diff.y()) / diff.x()) : 100.0;

    if (tanx > 10 && m_orientation == Qt::Horizontal) {
        m_orientation = Qt::Vertical;
    } else if (tanx < 0.1 && m_orientation == Qt::Vertical) {
        m_orientation = Qt::Horizontal;
    }

    // people like it more when the they can zoom by dragging in both directions
    Q_UNUSED(m_orientation);

    value = diff.x() - diff.y();

    return value;
}

int KisDraggableToolButton::movementDelta(const QPoint &pos)
{
    QPoint diff = pos - m_lastPosition;
    m_lastPosition = pos;
    return diff.x() - diff.y();

}

void KisDraggableToolButton::mousePressEvent(QMouseEvent *e)
{
    beginDrag(e->pos());
    QToolButton::mousePressEvent(e);
}

void KisDraggableToolButton::mouseMoveEvent(QMouseEvent *e)
{
    int distance = continueDrag(e->pos());
    emit offsetChanged(distance);
    int delta = movementDelta(e->pos());
    emit valueChanged(delta);

    QToolButton::mouseMoveEvent(e);
}
