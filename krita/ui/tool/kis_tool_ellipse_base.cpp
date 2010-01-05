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

#include <opengl/kis_opengl.h>

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
    Q_ASSERT(m_canvas && currentImage());

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
    Q_ASSERT(m_canvas && currentImage());
    if (!currentNode()) return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        updateArea();
        m_dragging = false;
        finishEllipse(QRectF(m_dragStart, m_dragEnd).normalized());
        event->accept();
    }
}


void KisToolEllipseBase::paintEllipse(QPainter& gc, const QRect&)
{
    QPointF viewDragStart = pixelToView(m_dragStart);
    QPointF viewDragEnd = pixelToView(m_dragEnd);

#if defined(HAVE_OPENGL)
    if (isCanvasOpenGL()) {
        beginOpenGL();

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        glColor3f(0.501961, 1.0, 0.501961);

        int steps = 72;
        qreal x = (viewDragEnd.x() - viewDragStart.x()) * 0.5;
        qreal a = qAbs(x);
        qreal y = (viewDragEnd.y() - viewDragStart.y()) * 0.5;
        qreal b = qAbs(y);

        x += viewDragStart.x();
        y += viewDragStart.y();

// useful for debugging
#if 0
        glPointSize(20);
        glBegin(GL_POINTS);
        glVertex2d(x, y);
        glEnd();

        glBegin(GL_LINES);
        glVertex2d(viewDragStart.x(), viewDragStart.y());
        glVertex2d(viewDragEnd.x(), viewDragEnd.y());

        glVertex2d(x, y);
        glVertex2d(x + a, y);

        glVertex2d(x, y);
        glVertex2d(x, y + b);

        glEnd();
#endif

        qreal angle = 0;
        qreal beta = -angle;
        qreal sinbeta = sin(beta);
        qreal cosbeta = cos(beta);

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 360; i += 360.0 / steps) {
            qreal alpha = i * (M_PI / 180) ;
            qreal sinalpha = sin(alpha);
            qreal cosalpha = cos(alpha);

            qreal X = x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta);
            qreal Y = y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta);

            glVertex2d(X, Y);
        }
        glEnd();


        glDisable(GL_COLOR_LOGIC_OP);
        glDisable(GL_LINE_SMOOTH);

         endOpenGL();
    } else
#endif

        if (m_canvas) {
#ifdef INDEPENDENT_CANVAS
            QPainterPath path;
            path.addEllipse(QRectF(viewDragStart, viewDragEnd));
            paintToolOutline(&gc, path);
#else
            QPen old = gc.pen();
            QPen pen(Qt::SolidLine);
            gc.setPen(pen);
            gc.drawEllipse(QRectF(viewDragStart, viewDragEnd));
            gc.setPen(old);
#endif
        }
}

void KisToolEllipseBase::updateArea()
{
    m_canvas->updateCanvas(convertToPt(QRectF(m_dragStart, m_dragEnd).normalized()));
}

#include "kis_tool_ellipse_base.moc"

