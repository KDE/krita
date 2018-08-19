/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
