/*
 *  kis_tool_polyline.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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


#include <math.h>

#include <QPainter>
#include <QSpinBox>
#include <QKeyEvent>
#include <QMenu>

#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "KoCanvasBase.h"
#include "kis_painter.h"
#include "KoPointerEvent.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"

#include "kis_tool_polyline.h"

KisToolPolyline::KisToolPolyline(KoCanvasBase * canvas)
        : super(canvas, KisCursor::load("tool_polyline_cursor.png", 6, 6)),
          m_dragging (false)
{
    setObjectName("tool_polyline");
}

KisToolPolyline::~KisToolPolyline()
{
}

void KisToolPolyline::mousePressEvent(KoPointerEvent *event)
{
    if (m_currentImage) {
        if (event->button() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier ) {

            m_dragging = true;

            if (m_points.isEmpty())
            {
                m_dragStart = convertToPixelCoord(event);
                m_dragEnd = convertToPixelCoord(event);
                m_points.append(m_dragStart);
                m_boundingRect = QRectF(m_dragStart.x(), m_dragStart.y(), 0, 0);
            } else {
                m_dragStart = m_dragEnd;
                m_dragEnd = convertToPixelCoord(event);
            }
        } else if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier ) {
            finish();
        }
        if (event->button() == Qt::RightButton && (m_dragging || !m_points.isEmpty()) ) {
            QMenu menu;
            menu.addAction(i18n("&Finish Polyline"), this, SLOT(finish()));
            menu.addAction(KIcon("cancel"), i18n("&Cancel"), this, SLOT(cancel()));
            menu.exec(QCursor::pos());
        }
    }
}

void KisToolPolyline::deactivate()
{
    m_points.clear();
    m_dragging = false;
}

void KisToolPolyline::finish()
{
    m_dragging = false;

    KisPaintDeviceSP device = m_currentImage->activeDevice ();
    if (!device) return;

    KisPainter painter (device);
    painter.beginTransaction (i18n ("Polyline"));

    painter.setPaintColor(m_currentFgColor);
    painter.setBrush(m_currentBrush);
    painter.setOpacity(m_opacity);
    painter.setCompositeOp(m_compositeOp);
    KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_currentPaintOp, m_currentPaintOpSettings, &painter);
    painter.setPaintOp(op); // Painter takes ownership

    QPointF start,end;
    KoPointVector::iterator it;
    for( it = m_points.begin(); it != m_points.end(); ++it )
    {
        if( it == m_points.begin() )
        {
            start = (*it);
        } else {
            end = (*it);
            painter.paintLine(start, PRESSURE_DEFAULT, 0, 0, end, PRESSURE_DEFAULT, 0, 0);
            start = end;
        }
    }
    m_points.clear();

    device->setDirty( painter.dirtyRegion() );
    notifyModified();

    m_canvas->addCommand(painter.endTransaction());
}

void KisToolPolyline::cancel()
{
       m_dragging = false;
       m_points.clear();
}

void KisToolPolyline::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        //Erase old lines
        m_canvas->updateCanvas(m_boundingRect.unite(QRectF(m_dragStart.x(), m_dragStart.y(), m_dragEnd.x() - m_dragStart.x(), m_dragEnd.y() - m_dragStart.y())));

        // get current mouse position
        m_dragEnd = convertToPixelCoord(event);
    }
    m_canvas->updateCanvas(m_boundingRect.unite(QRectF(m_dragStart.x(), m_dragStart.y(), m_dragEnd.x() - m_dragStart.x(), m_dragEnd.y() - m_dragStart.y())));
}

void KisToolPolyline::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas || !m_currentImage)
            return;

    if (m_dragging && event->button() == Qt::LeftButton)  {
        m_dragging = false;
        m_points.append (m_dragEnd);
        m_boundingRect = m_boundingRect.unite(QRectF(m_dragStart.x(), m_dragStart.y(), m_dragEnd.x() - m_dragStart.x(), m_dragEnd.y() -m_dragStart.y()));
    }
    m_canvas->updateCanvas(m_boundingRect);
}


void KisToolPolyline::mouseDoubleClickEvent(KoPointerEvent *)
{
    finish();
}

void KisToolPolyline::paint(QPainter& gc, KoViewConverter &converter)
{
    if (!m_canvas || !m_currentImage)
        return;

    double sx, sy;
    converter.zoom(&sx, &sy);

    gc.scale( sx/m_currentImage->xRes(), sy/m_currentImage->yRes() );


    QPen pen( Qt::SolidLine);
    gc.setPen(pen);
    //gc.setRasterOp(Qt::XorROP);

    QPointF start, end;
    QPointF startPos;
    QPointF endPos;

    if (m_dragging) {
        startPos = m_dragStart;
        endPos = m_dragEnd;
        gc.drawLine(startPos, endPos);
    }
    for (KoPointVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

        if (it == m_points.begin())
        {
            start = (*it);
        } else {
            end = (*it);

            startPos = start;
            endPos = end;

            gc.drawLine(startPos, endPos);

            start = end;
        }
    }
}

QString KisToolPolyline::quickHelp() const
{
    return i18n("Press shift-mouseclick to end the polyline.");
}

void KisToolPolyline::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Escape)
       cancel();
}

#include "kis_tool_polyline.moc"
