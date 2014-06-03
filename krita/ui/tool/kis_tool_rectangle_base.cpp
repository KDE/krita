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

#include <QtCore/qmath.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>


KisToolRectangleBase::KisToolRectangleBase(KoCanvasBase * canvas, KisToolRectangleBase::ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor)
    , m_dragStart(0, 0)
    , m_dragEnd(0, 0)
    , m_type(type)
{
}

void KisToolRectangleBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    if(mode() == KisTool::PAINT_MODE) {
        paintRectangle(gc, createRect(m_dragStart, m_dragEnd));
    }

    KisToolPaint::paint(gc, converter);
}

void KisToolRectangleBase::deactivate()
{
    updateArea();
    KisToolShape::deactivate();
}

void KisToolRectangleBase::beginPrimaryAction(KoPointerEvent *event)
{
    if ((m_type == PAINT && (!nodeEditable() || nodePaintAbility() == NONE)) ||
        (m_type == SELECT && !selectionEditable())) {

        event->ignore();
        return;
    }
    setMode(KisTool::PAINT_MODE);

    QPointF pos = convertToPixelCoord(event);
    m_dragStart = m_dragCenter = m_dragEnd = pos;
    event->accept();
}

void KisToolRectangleBase::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPointF pos = convertToPixelCoord(event);

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
    KisToolPaint::requestUpdateOutline(event->point, event);
}

void KisToolRectangleBase::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    updateArea();

    finishRect(createRect(m_dragStart, m_dragEnd));
    event->accept();
}

QRectF KisToolRectangleBase::createRect(const QPointF &start, const QPointF &end)
{
    /**
     * To make the dragging user-friendly it should work in a bit
     * non-obvious way: the start-drag point must be handled with
     * "ceil"/"floor" (depending on the direction of the drag) and the
     * end-drag point should follow usual "round" semantics.
     */

    qreal x0 = start.x();
    qreal y0 = start.y();
    qreal x1 = end.x();
    qreal y1 = end.y();

    int newX0 = x1 - x0 > 0 ? qCeil(x0) : qFloor(x0);
    int newY0 = y1 - y0 > 0 ? qCeil(y0) : qFloor(y0);

    int newX1 = qRound(x1);
    int newY1 = qRound(y1);

    QRectF result;
    result.setCoords(newX0, newY0, newX1, newY1);
    return result.normalized();
}

void KisToolRectangleBase::paintRectangle(QPainter &gc, const QRectF &imageRect)
{
    KIS_ASSERT_RECOVER_RETURN(canvas());

    QRect viewRect = pixelToView(imageRect).toAlignedRect();

    QPainterPath path;
    path.addRect(viewRect);
    paintToolOutline(&gc, path);
}

void KisToolRectangleBase::updateArea() {
    QRectF bound;
    bound.setTopLeft(m_dragStart);
    bound.setBottomRight(m_dragEnd);
    canvas()->updateCanvas(convertToPt(bound.normalized()).adjusted(-100, -100, +200, +200));
}

#include "kis_tool_rectangle_base.moc"
