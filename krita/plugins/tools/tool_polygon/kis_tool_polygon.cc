/*
 *  kis_tool_polygon.cc -- part of Krita
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

#include <kaction.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>

#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"

#include "kis_tool_polygon.h"

KisToolPolygon::KisToolPolygon(KoCanvasBase *canvas)
        : super(canvas, KisCursor::load("tool_polygon_cursor.png", 6, 6)),
          m_dragging (false)
{
    setObjectName("tool_polygon");
}

KisToolPolygon::~KisToolPolygon()
{
}

void KisToolPolygon::mousePressEvent(KoPointerEvent *event)
{
    if (m_currentImage) {
        if (event->button() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier) {

            m_dragging = true;

            if (m_points.isEmpty())
            {
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
    }
}

void KisToolPolygon::mouseMoveEvent(KoPointerEvent *event)
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

void KisToolPolygon::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas || !m_currentImage)
        return;

    if (m_dragging && event->button() == Qt::LeftButton)  {
            m_dragging = false;
            m_points.append (m_dragEnd);
    }

    if (m_dragging && event->button() == Qt::RightButton) {

        }
}

void KisToolPolygon::mouseDoubleClickEvent( KoPointerEvent * )
{
    finish();
}

void KisToolPolygon::keyPressEvent(QKeyEvent *e)
{
    if (e->key()==Qt::Key_Escape)
    {
        m_dragging = false;
        m_points.clear();
        // erase old lines on canvas
        updateCanvasPixelRect(image()->bounds());
    }
}

void KisToolPolygon::finish()
{
    m_dragging = false;

    KisPaintDeviceSP device = m_currentImage->activeDevice ();
    if (device) {
        KisPainter painter (device);
        if (m_currentImage->undo()) painter.beginTransaction (i18n ("Polygon"));

        painter.setPaintColor(m_currentFgColor);
        painter.setBackgroundColor(m_currentBgColor);
        painter.setFillStyle(fillStyle());
        painter.setBrush(m_currentBrush);
        painter.setPattern(m_currentPattern);
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_currentPaintOp, 
                                                                  m_currentPaintOpSettings, 
                                                                  &painter);
        painter.setPaintOp(op); // Painter takes ownership

        painter.paintPolygon(m_points);
        device->setDirty( painter.dirtyRegion() );
        notifyModified();

        m_canvas->addCommand(painter.endTransaction());
    }

    m_points.clear();

    // erase old lines on canvas
    updateCanvasPixelRect(image()->bounds());
}

#define PREVIEW_LINE_WIDTH 1

void KisToolPolygon::paint(QPainter& gc, KoViewConverter &converter)
{
    Q_UNUSED(converter);

    if (!m_canvas || !m_currentImage)
        return;

    gc.save();

    QPen pen(Qt::SolidLine);
    pen.setWidth(PREVIEW_LINE_WIDTH);
    gc.setPen(pen);

    QPointF start, end;
    QPointF startPos;
    QPointF endPos;

    if (m_dragging) {
        startPos = pixelToView(m_dragStart);
        endPos = pixelToView(m_dragEnd);
        gc.drawLine(startPos, endPos);
    }
    for (vQPointF::iterator it = m_points.begin(); it != m_points.end(); ++it) {

        if (it == m_points.begin())
        {
            start = (*it);
        } else {
            end = (*it);

            startPos = pixelToView(start);
            endPos = pixelToView(end);
            gc.drawLine(startPos, endPos);
            start = end;
        }
    }
    gc.restore();
}

QRectF KisToolPolygon::dragBoundingRect()
{
    QRectF rect = pixelToView(QRectF(m_dragStart, m_dragEnd).normalized());
    rect.adjust(-PREVIEW_LINE_WIDTH, -PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH, PREVIEW_LINE_WIDTH);
    return rect;
}

#include "kis_tool_polygon.moc"
