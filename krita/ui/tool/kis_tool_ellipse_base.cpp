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

KisToolEllipseBase::KisToolEllipseBase(KoCanvasBase * canvas, KisToolEllipseBase::ToolType type, const QCursor & cursor) :
        KisToolShape(canvas, cursor),
        m_dragStart(0,0),
        m_dragEnd(0,0),
        m_type(type)
{
}

KisToolEllipseBase::~KisToolEllipseBase()
{
}

void KisToolEllipseBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    Q_ASSERT(currentImage());
    if (mode() == KisTool::PAINT_MODE)
        paintEllipse(gc, QRect());
}

void KisToolEllipseBase::deactivate()
{
    updateArea();
    KisToolShape::deactivate();
}


void KisToolEllipseBase::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        if (m_type == PAINT) {
            if (!nodeEditable() || nodePaintAbility() == NONE) {
                return;
            }
        } else {
            if (!selectionEditable()) {
                return;
            }
        }
        setMode(KisTool::PAINT_MODE);

        QPointF pos = convertToPixelCoord(event);
        m_dragStart = m_dragCenter = m_dragEnd = pos;
        event->accept();
    }
    else {
        KisToolShape::mousePressEvent(event);
    }
}

void KisToolEllipseBase::mouseMoveEvent(KoPointerEvent *event)
{
    if(MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
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
    else {
        KisToolShape::mouseMoveEvent(event);
    }
}

void KisToolEllipseBase::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        updateArea();

        finishEllipse(QRectF(m_dragStart, m_dragEnd).normalized());
        event->accept();
    }
    else {
        KisToolShape::mouseReleaseEvent(event);
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
    canvas()->updateCanvas(convertToPt(QRectF(m_dragStart, m_dragEnd).normalized().adjusted(-10, -10, 10, 10)));
}

#include "kis_tool_ellipse_base.moc"

