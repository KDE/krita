/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

int KisDraggableToolButton::unitRadius()
{
    return 200;
}

void KisDraggableToolButton::beginDrag(const QPoint &pos)
{
    m_startPoint = pos;
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

void KisDraggableToolButton::mousePressEvent(QMouseEvent *e)
{
    m_startPoint = e->pos();
    QToolButton::mousePressEvent(e);
}

void KisDraggableToolButton::mouseMoveEvent(QMouseEvent *e)
{
    int value = continueDrag(e->pos());
    emit valueChanged(value);

    QToolButton::mouseMoveEvent(e);
}
