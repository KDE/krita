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

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>


#include <opengl/kis_opengl.h>

#include "kis_tool_polygon_base.h"
#define PREVIEW_LINE_WIDTH 1

KisToolPolygonBase::KisToolPolygonBase(KoCanvasBase * canvas, const QCursor & cursor) :
        KisToolShape(canvas, cursor),
        m_dragging(false)
{
}
void KisToolPolygonBase::mousePressEvent(KoPointerEvent *event)
{
    if (currentImage()) {
        if (event->button() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier) {

            m_dragging = true;

            if (m_points.isEmpty()) {
                m_dragStart = convertToPixelCoord(event);
                m_dragEnd = m_dragStart;
                m_points.append(m_dragStart);
            } else {
                m_dragStart = m_dragEnd;
                m_dragEnd = convertToPixelCoord(event);
            }
        } else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier) {
            finish();
        }
        else if (event->button()==Qt::RightButton || event->button()==Qt::MidButton) {
            cancel();
            event->accept();
        }
    }
}

void KisToolPolygonBase::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        // erase old lines on canvas
        QRectF updateRect = dragBoundingRect();
        // get current mouse position
        m_dragEnd = convertToPixelCoord(event);
        // draw new lines on canvas
        updateRect |= dragBoundingRect();
        updateCanvasViewRect(updateRect);
    }
}

void KisToolPolygonBase::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas || !currentImage())
        return;

    if (m_dragging && event->button() == Qt::LeftButton)  {
        m_dragging = false;
        m_points.append(m_dragEnd);
        updateArea();
    }

    if (m_dragging && event->button() == Qt::RightButton) {

    }
}

void KisToolPolygonBase::mouseDoubleClickEvent(KoPointerEvent *)
{
    finish();
}

void KisToolPolygonBase::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        cancel();
    }
}

void KisToolPolygonBase::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (!m_canvas || !currentImage())
        return;

    QPointF start, end;
    QPointF startPos;
    QPointF endPos;

#if defined(HAVE_OPENGL)
    if (isCanvasOpenGL()) {
        beginOpenGL();

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        glColor3f(0.501961, 1.0, 0.501961);

        if (m_dragging) {
            startPos = pixelToView(m_dragStart);
            endPos = pixelToView(m_dragEnd);
            glBegin(GL_LINES);
            glVertex2f(startPos.x(), startPos.y());
            glVertex2f(endPos.x(), endPos.y());
            glEnd();
        }

        glBegin(GL_LINES);
        for (vQPointF::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin()) {
                start = (*it);
            } else {
                end = (*it);

                startPos = pixelToView(start);
                endPos = pixelToView(end);

                glVertex2f(startPos.x(), startPos.y());
                glVertex2f(endPos.x(), endPos.y());

                start = end;
            }
        }
        glEnd();

        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_LINE_SMOOTH);

        endOpenGL();
    } else
#endif

#ifdef INDEPENDENT_CANVAS
    {
        QPainterPath path;
        if (m_dragging) {
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
#else
    {
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        pen.setWidth(PREVIEW_LINE_WIDTH);
        gc.setPen(pen);

        if (m_dragging) {
            startPos = pixelToView(m_dragStart);
            endPos = pixelToView(m_dragEnd);
            gc.drawLine(startPos, endPos);
        }

        for (vQPointF::iterator it = m_points.begin(); it != m_points.end(); ++it) {

            if (it == m_points.begin()) {
                start = (*it);
            } else {
                end = (*it);

                startPos = pixelToView(start);
                endPos = pixelToView(end);
                gc.drawLine(startPos, endPos);
                start = end;
            }
        }
        gc.setPen(old);

    }
#endif
}

void KisToolPolygonBase::cancel()
{
    m_dragging = false;
    m_points.clear();
    updateArea();
}

void KisToolPolygonBase::updateArea()
{
    updateCanvasPixelRect(image()->bounds());
}

void KisToolPolygonBase::finish()
{
    Q_ASSERT(m_canvas && currentImage());
    if (!currentNode())
        return;

    m_dragging = false;
    updateArea();
    finishPolygon(m_points);
    m_points.clear();
}

QRectF KisToolPolygonBase::dragBoundingRect()
{
    QRectF rect = pixelToView(QRectF(m_dragStart, m_dragEnd).normalized());
    rect.adjust(-PREVIEW_LINE_WIDTH, -PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH);
    return rect;
}

#include "kis_tool_polygon_base.moc"
