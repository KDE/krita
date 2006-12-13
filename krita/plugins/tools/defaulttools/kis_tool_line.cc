/*
 *  kis_tool_line.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2003 Boudewijn Rempt <boud@valdyas.org>
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

#include <QPainter>
#include <QLayout>
#include <QWidget>

#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_tool_line.h"
#include "KoPointerEvent.h"
#include "kis_paintop_registry.h"
#include "kis_undo_adapter.h"
#include "QPainter"
#include "kis_cursor.h"
#include "kis_layer.h"
#include "KoCanvasBase.h"

KisToolLine::KisToolLine(KoCanvasBase * canvas)
    : KisToolPaint(canvas, KisCursor::load("tool_line_cursor.png", 6, 6)),
      m_dragging( false )
{
    setObjectName("tool_line");


    m_painter = 0;
    m_currentImage = 0;
    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);
}

KisToolLine::~KisToolLine()
{
}

void KisToolLine::paint(QPainter& gc)
{
    if (m_dragging)
        paintLine(gc, QRect());
}

void KisToolLine::paint(QPainter& gc, const QRect& rc)
{
    if (m_dragging)
        paintLine(gc, rc);
}

void KisToolLine::buttonPress(KoPointerEvent *e)
{
    if (!m_canvas || !m_currentImage) return;

    if (!m_currentBrush) return;

    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_startPos = e->pos(); //controller->windowToView(e->pos());
        m_endPos = e->pos(); //controller->windowToView(e->pos());
    }
}

void KisToolLine::move(KoPointerEvent *e)
{
    if (m_dragging) {
        if (m_startPos != m_endPos)
            paintLine();

        if (e->modifiers() & Qt::AltModifier) {
            QPointF trans = e->pos() - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        } else if (e->modifiers() & Qt::ShiftModifier)
            m_endPos = straightLine(e->pos());
        else
            m_endPos = e->pos();//controller->windowToView(e->pos());
        paintLine();
    }
}

void KisToolLine::buttonRelease(KoPointerEvent *e)
{
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;

        if(m_canvas) {

            if (m_startPos == m_endPos) {
                /* FIXME Which rectangle to repaint */
                //m_canvas->updateCanvas();
                m_dragging = false;
                return;
            }

            if ((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) {
                m_endPos = straightLine(e->pos());
            } else m_endPos = e->pos();

            KisPaintDeviceSP device;
            if (m_currentImage &&
                (device = m_currentImage->activeDevice()) &&
                m_currentBrush) {
                delete m_painter;
                m_painter = new KisPainter( device );
                Q_CHECK_PTR(m_painter);

                if (m_currentImage->undo()) m_painter->beginTransaction(i18n("Line"));

                m_painter->setPaintColor(m_currentFgColor);
                m_painter->setBrush(m_currentBrush);
                m_painter->setOpacity(m_opacity);
                m_painter->setCompositeOp(m_compositeOp);
                KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_currentPaintOp, m_currentPaintOpSettings, m_painter);
                m_painter->setPaintOp(op); // Painter takes ownership
                m_painter->paintLine(m_startPos, PRESSURE_DEFAULT, 0, 0, m_endPos, PRESSURE_DEFAULT, 0, 0);
                device->setDirty( m_painter->dirtyRect() );
                notifyModified();

		/* FIXME Which rectangle to repaint */
                //if (m_canvas) {
                //    m_canvas->updateCanvas();
                //}

                if (m_currentImage->undo() && m_painter) {
                    m_currentImage->undoAdapter()->addCommand(m_painter->endTransaction());
                }
                delete m_painter;
                m_painter = 0;
            } else {
                // m_painter can be 0 here...!!!
                m_canvas->updateCanvas(m_painter->dirtyRect()); // Removes the last remaining line.
            }
        }
    }

}


QPointF KisToolLine::straightLine(QPointF point)
{
    QPointF comparison = point - m_startPos;
    QPointF result;

    if ( fabs(comparison.x()) > fabs(comparison.y())) {
        result.setX(point.x());
        result.setY(m_startPos.y());
    } else {
        result.setX( m_startPos.x() );
        result.setY( point.y() );
    }

    return result;
}

void KisToolLine::paintLine()
{
    if (m_canvas) {
        QPainter gc(m_canvas->canvasWidget());
        QRect rc;

        paintLine(gc, rc);
    }
}

void KisToolLine::paintLine(QPainter& gc, const QRect&)
{
    if (m_canvas) {
        //KisCanvasController *controller = m_subject->canvasController();
        //RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        QPointF start;
        QPointF end;

//        Q_ASSERT(controller);
        //start = m_canvas->windowToView(m_startPos);
        //end = m_canvas->windowToView(m_endPos);
	start = m_startPos;
	end = m_endPos;
//          start.setX(start.x() - controller->horzValue());
//          start.setY(start.y() - controller->vertValue());
//          end.setX(end.x() - controller->horzValue());
//          end.setY(end.y() - controller->vertValue());
//          end.setX((end.x() - start.x()));
//          end.setY((end.y() - start.y()));
//         start *= m_subject->zoomFactor();
//         end *= m_subject->zoomFactor();
        //gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        //gc.drawLine(start.toPoint(), end.toPoint());
	start = QPoint(static_cast<int>(start.x()), static_cast<int>(start.y()));
	end = QPoint(static_cast<int>(end.x()), static_cast<int>(end.y()));
	gc.drawLine(start, end);
	//gc.setRasterOp(op);
        gc.setPen(old);
    }
}


QString KisToolLine::quickHelp() const {
    return i18n("Alt+Drag will move the origin of the currently displayed line around, Shift+Drag will force you to draw straight lines");
}

#include "kis_tool_line.moc"

