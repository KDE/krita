/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tool_rectangle_base.h"
#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>


KisToolRectangleBase::KisToolRectangleBase(KoCanvasBase * canvas, const QCursor & cursor) :
    KisToolShape(canvas, cursor), m_dragStart(0, 0), m_dragEnd(0, 0), m_dragging(false)
{
}

void KisToolRectangleBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    Q_ASSERT(currentImage());

    if (m_dragging)
        paintRectangle(gc, QRect());
}

void KisToolRectangleBase::deactivate()
{
    m_dragging=false;
    updateArea();
}


void KisToolRectangleBase::mousePressEvent(KoPointerEvent *e)
{
    Q_ASSERT(canvas() && currentImage());
    
    if (!currentNode() || currentNode()->systemLocked()) {
        return;
    }

    if (e->button() == Qt::LeftButton) {
        QPointF pos = convertToPixelCoord(e);
        m_dragging = true;
        m_dragStart = m_dragCenter = m_dragEnd = pos;
        e->accept();
    }
    else if (e->button()==Qt::RightButton || e->button()==Qt::MidButton) {
        m_dragging = false;
        updateArea();
        e->accept();
    }
}

void KisToolRectangleBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        QPointF pos = convertToPixelCoord(event);
        updateArea();

        // move (alt) or resize rectangle
        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = pos - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            QPointF diag = pos - (event->modifiers() & Qt::ControlModifier
                                  ? m_dragCenter : m_dragStart);
            // square?
            if (event->modifiers() & Qt::ShiftModifier) {
                double size = qMax(fabs(diag.x()), fabs(diag.y()));
                double w = diag.x() < 0 ? -size : size;
                double h = diag.y() < 0 ? -size : size;
                diag = QPointF(w, h);
            }

            // resize around center point?
            if (event->modifiers() & Qt::ControlModifier) {
                m_dragStart = m_dragCenter - diag;
                m_dragEnd = m_dragCenter + diag;
            } else {
                m_dragEnd = m_dragStart + diag;
            }
        }

        updateArea();

        m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                               (m_dragStart.y() + m_dragEnd.y()) / 2);
    } else {
        KisToolPaint::mouseReleaseEvent(event);
    }
}

void KisToolRectangleBase::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_ASSERT(canvas() && currentImage());
    if (!currentNode()) return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        updateArea();
        m_dragging = false;
        setCurrentNodeLocked(true);
        finishRect(QRectF(m_dragStart, m_dragEnd));
        setCurrentNodeLocked(false);
        event->accept();
    }
}


void KisToolRectangleBase::paintRectangle(QPainter& gc, const QRect&)
{
    Q_ASSERT(canvas() && currentImage());

    QPainterPath path;
    path.addRect(QRectF(pixelToView(m_dragStart), pixelToView(m_dragEnd)));
    paintToolOutline(&gc, path);
}

void KisToolRectangleBase::updateArea() {
    QRectF bound;
    bound.setTopLeft(m_dragStart);
    bound.setBottomRight(m_dragEnd);
    canvas()->updateCanvas(convertToPt(bound.normalized()));
}

#include "kis_tool_rectangle_base.moc"
