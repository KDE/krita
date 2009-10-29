/*
 *  kis_tool_ellipse.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_tool_ellipse.h"


#include <QPainter>

#include <kis_debug.h>
#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShapeController.h>

#include <kis_selection.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_cursor.h>
#include <kis_layer.h>
#include <kis_shape_tool_helper.h>


#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#endif



KisToolEllipse::KisToolEllipse(KoCanvasBase * canvas)
        : KisToolShape(canvas, KisCursor::load("tool_ellipse_cursor.png", 6, 6)),
        m_dragging(false)
{
    setObjectName("tool_ellipse");

    m_painter = 0;
    currentImage() = 0;
    m_dragStart = QPointF(0, 0);
    m_dragEnd = QPointF(0, 0);
}

KisToolEllipse::~KisToolEllipse()
{
}

void KisToolEllipse::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_ASSERT(currentImage());
    if (m_dragging)
        paintEllipse(gc, QRect());
}


void KisToolEllipse::mousePressEvent(KoPointerEvent *event)
{
    if (!m_canvas || !currentImage()) return;

    if (event->button() == Qt::LeftButton) {
        QPointF pos = convertToPixelCoord(event);
        m_dragging = true;
        m_dragStart = m_dragCenter = m_dragEnd = pos;
        //draw(m_dragStart, m_dragEnd);
    }
}

void KisToolEllipse::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        QRectF bound;

        bound.setTopLeft(m_dragStart);
        bound.setBottomRight(m_dragEnd);
        m_canvas->updateCanvas(convertToPt(bound.normalized()));

        QPointF pos = convertToPixelCoord(event);
        // erase old lines on canvas
        //draw(m_dragStart, m_dragEnd);
        // move (alt) or resize ellipse
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
        // draw new lines on canvas
        //draw(m_dragStart, m_dragEnd);
        bound.setTopLeft(m_dragStart);
        bound.setBottomRight(m_dragEnd);

        m_canvas->updateCanvas(convertToPt(bound.normalized()));

        m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                               (m_dragStart.y() + m_dragEnd.y()) / 2);
    }
}

void KisToolEllipse::mouseReleaseEvent(KoPointerEvent *event)
{
    QPointF pos = convertToPixelCoord(event);

    if (!m_canvas || !currentImage())
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        // erase old lines on canvas
        //draw(m_dragStart, m_dragEnd);
        m_dragging = false;

        if (m_dragStart == m_dragEnd)
            return;

        if (!currentImage())
            return;

        if (!currentNode())
            return;

        if (!currentNode()->inherits("KisShapeLayer")) {
            if (!currentNode()->paintDevice())
                return;

            KisPaintDeviceSP device = currentNode()->paintDevice();
            delete m_painter;
            m_painter = new KisPainter(device, currentSelection());
            Q_CHECK_PTR(m_painter);

            m_painter->beginTransaction(i18n("Ellipse"));
            setupPainter(m_painter);
            m_painter->setOpacity(m_opacity);
            m_painter->setCompositeOp(m_compositeOp);

            m_painter->paintEllipse(QRectF(m_dragStart, m_dragEnd));
            QRegion bound = m_painter->dirtyRegion();
            device->setDirty(bound);
            notifyModified();

            m_canvas->addCommand(m_painter->endTransaction());

            delete m_painter;
            m_painter = 0;
        } else {
            QRectF rect = convertToPt(QRectF(m_dragStart, m_dragEnd));
            KoShape* shape = KisShapeToolHelper::createEllipseShape(rect);

            QUndoCommand * cmd = m_canvas->shapeController()->addShape(shape);
            m_canvas->addCommand(cmd);
         }
    } else {
        KisToolPaint::mouseReleaseEvent(event);
    }
}


void KisToolEllipse::paintEllipse(QPainter& gc, const QRect&)
{
    QPointF viewDragStart = pixelToView(m_dragStart);
    QPointF viewDragEnd = pixelToView(m_dragEnd);

#if defined(HAVE_OPENGL)
    if (m_canvas->canvasController()->isCanvasOpenGL()) {
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

#include "kis_tool_ellipse.moc"
