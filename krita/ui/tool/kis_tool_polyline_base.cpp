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

#include <QList>

#include <kaction.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>

#include "kis_tool_polyline_base.h"

#define PREVIEW_LINE_WIDTH 1

KisToolPolylineBase::KisToolPolylineBase(KoCanvasBase * canvas,  KisToolPolylineBase::ToolType type, const QCursor & cursor)
    : KisToolShape(canvas, cursor),
      m_dragging(false),
      m_type(type)
{
}

void KisToolPolylineBase::mousePressEvent(KoPointerEvent *event)
{
    if (m_type == PAINT) {
        if (!nodeEditable() || nodePaintAbility() == NONE) {
            return;
        }
    } else {
        if (!selectionEditable()) {
            return;
        }
    }
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier)) {

        if(event->modifiers() == Qt::ShiftModifier) {
            finish();
        }
        else {
            setMode(KisTool::PAINT_MODE);
            m_dragging = true;
        }
    }
    else {
        KisToolShape::mousePressEvent(event);
    }
}

void KisToolPolylineBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging && !m_points.empty()) {
        // erase old lines on canvas
        QRectF updateRect = dragBoundingRect();
        // get current mouse position
        m_dragEnd = convertToPixelCoord(event);
        // draw new lines on canvas
        updateRect |= dragBoundingRect();
        updateCanvasViewRect(updateRect);
    }

    KisToolShape::mouseMoveEvent(event);
}

void KisToolPolylineBase::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        if(m_dragging) {
            m_dragStart = convertToPixelCoord(event);
            m_dragEnd = m_dragStart;
            m_points.append(m_dragStart);
        }
    }
    else {
        KisToolShape::mouseReleaseEvent(event);
    }
}

void KisToolPolylineBase::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION(event, KisTool::HOVER_MODE,
                       Qt::LeftButton, Qt::NoModifier)) {

        finish();
    }
    else {
        KisToolShape::mousePressEvent(event);
    }
}

void KisToolPolylineBase::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        cancel();
        e->accept();
    }

    KisToolShape::keyPressEvent(e);
}

void KisToolPolylineBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (!canvas() || !currentImage())
        return;

    QPointF start, end;
    QPointF startPos;
    QPointF endPos;

    QPainterPath path;
    if (m_dragging && !m_points.empty()) {
        startPos = pixelToView(m_dragStart);
        endPos = pixelToView(m_dragEnd);
        path.moveTo(startPos);
        path.lineTo(endPos);
    }

    for (vQPointF::iterator it = m_points.begin(); it != m_points.end(); ++it) {

        if (it == m_points.begin()) {
            start = (*it);
        } else {
            end = (*it);

            startPos = pixelToView(start);
            endPos = pixelToView(end);
            path.moveTo(startPos);
            path.lineTo(endPos);
            start = end;
        }
    }
    paintToolOutline(&gc, path);
}

void KisToolPolylineBase::cancel()
{
    m_dragging = false;
    m_points.clear();
    updateArea();
}

void KisToolPolylineBase::updateArea()
{
    updateCanvasPixelRect(image()->bounds());
}

void KisToolPolylineBase::finish()
{
    Q_ASSERT(canvas() && currentImage());
    if (!currentNode())
        return;

    m_dragging = false;
    updateArea();
    if(m_points.count() > 1) {
        finishPolyline(m_points);
    }
    m_points.clear();
}

QRectF KisToolPolylineBase::dragBoundingRect()
{
    QRectF rect = pixelToView(QRectF(m_dragStart, m_dragEnd).normalized());
    rect.adjust(-PREVIEW_LINE_WIDTH, -PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH);
    return rect;
}

#include "kis_tool_polyline_base.moc"
