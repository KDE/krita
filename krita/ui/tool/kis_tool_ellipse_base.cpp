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

#include "kis_tool_ellipse_base.h"

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>

#include "kis_canvas2.h"

KisToolEllipseBase::KisToolEllipseBase(KoCanvasBase * canvas, const QCursor & cursor) :
        KisToolShape(canvas, cursor),
        m_dragStart(0,0),
        m_dragEnd(0,0),
        m_dragging(false)
{
}

KisToolEllipseBase::~KisToolEllipseBase()
{
}

void KisToolEllipseBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    Q_ASSERT(currentImage());
    if (m_dragging)
        paintEllipse(gc, QRect());
}

void KisToolEllipseBase::deactivate()
{
    m_dragging=false;
    updateArea();
}


void KisToolEllipseBase::mousePressEvent(KoPointerEvent *event)
{
    Q_ASSERT(canvas() && currentImage());

    if (!currentNode() || currentNode()->systemLocked()) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        QPointF pos = convertToPixelCoord(event);
        m_dragging = true;
        m_dragStart = m_dragCenter = m_dragEnd = pos;
        event->accept();
    }
    else if (event->button()==Qt::RightButton || event->button()==Qt::MidButton) {
        m_dragging = false;
        updateArea();
        event->accept();
    }
}

void KisToolEllipseBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        updateArea();

        QPointF pos = convertToPixelCoord(event);

        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = pos - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            QPointF diag = pos - (event->modifiers() & Qt::ControlModifier
                                  ? m_dragCenter : m_dragStart);
            // circle?
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
    }
}

void KisToolEllipseBase::mouseReleaseEvent(KoPointerEvent *event)
{
    Q_ASSERT(canvas() && currentImage());
    if (!currentNode()) return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        updateArea();
        m_dragging = false;
        setCurrentNodeLocked(true);
        finishEllipse(QRectF(m_dragStart, m_dragEnd).normalized());
        setCurrentNodeLocked(false);
        event->accept();
    }
}


void KisToolEllipseBase::paintEllipse(QPainter& gc, const QRect&)
{
    if (canvas()) {
        QPainterPath path;
        path.addEllipse(QRectF(pixelToView(m_dragStart), pixelToView(m_dragEnd)));
        paintToolOutline(&gc, path);
    }
}

void KisToolEllipseBase::updateArea()
{
    canvas()->updateCanvas(convertToPt(QRectF(m_dragStart, m_dragEnd).normalized()));
}

#include "kis_tool_ellipse_base.moc"

