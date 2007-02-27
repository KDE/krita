/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>
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

#include <kaction.h>
#include <klocale.h>
#include <kactioncollection.h>

#include "QPainter"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "ZoomTool.h"


ZoomTool::ZoomTool(KoCanvasBase *canvas)
    : super(canvas)
{
    setObjectName("tool_zoom");
    m_dragging = false;
    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);
//    m_plusCursor = KisCursor::load("tool_zoom_plus_cursor.png", 8, 8);
//    m_minusCursor = KisCursor::load("tool_zoom_minus_cursor.png", 8, 8);
//    setCursor(m_plusCursor);
    connect(&m_timer, SIGNAL(timeout()), SLOT(slotTimer()));
}

ZoomTool::~ZoomTool()
{
}

void ZoomTool::mousePressEvent(KoPointerEvent *e)
{
    if (!m_dragging) {
        if (e->button() == Qt::LeftButton) {
            m_startPos = e->pos();
            m_endPos = e->pos();
            m_dragging = true;
        }
    }
}

void ZoomTool::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_dragging) {
	QRectF bound;
        bound.setTopLeft(m_startPos);
        bound.setBottomRight(m_endPos);
        m_canvas->updateCanvas(bound.normalized());
        m_endPos = e->pos();
        bound.setBottomRight(m_endPos);
        m_canvas->updateCanvas(bound.normalized());
    }
}

void ZoomTool::mouseReleaseEvent(KoPointerEvent *e)
{/*
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_endPos = e->pos();
        m_dragging = false;

        QPointF delta = m_endPos - m_startPos;

        if (sqrt(delta.x() * delta.x() + delta.y() * delta.y()) < 10) {
            if (e->modifiers() & Qt::ControlModifier) {
                controller->zoomOut(m_endPos.x(), m_endPos.y());
            } else {
                controller->zoomIn(m_endPos.x(), m_endPos.y());
            }
        } else {
            controller->zoomTo(QRect(m_startPos, m_endPos));
        }
    }*/
}

void ZoomTool::mouseDoubleClickEvent(KoPointerEvent *e)
{
}


void ZoomTool::activate()
{
    super::activate();
    m_timer.start(50);
}

void ZoomTool::deactivate()
{
    m_timer.stop();
}

void ZoomTool::slotTimer()
{
/*    int state = QApplication::keyboardModifiers() & (Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier);

    if (state & Qt::ControlModifier) {
        m_subject->canvasController()->setCanvasCursor(m_minusCursor);
    } else {
        m_subject->canvasController()->setCanvasCursor(m_plusCursor);
    }
*/
}


void ZoomTool::paint(QPainter &painter, KoViewConverter &converter)
{
    if (m_canvas) {
        QPen old = painter.pen();
        QPen pen(Qt::DotLine);
        QPoint start;
        QPoint end;

        start = QPoint(static_cast<int>(m_startPos.x()), static_cast<int>(m_startPos.y()));
        end = QPoint(static_cast<int>(m_endPos.x()), static_cast<int>(m_endPos.y()));
        painter.drawRect(QRect(start, end));
        painter.setPen(old);
    }
}

#include "ZoomTool.moc"
