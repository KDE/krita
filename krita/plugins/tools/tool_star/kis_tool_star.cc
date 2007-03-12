/*
 *  kis_tool_star.cc -- part of Krita
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
#include <QLayout>
#include <QGridLayout>

#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "KoCanvasBase.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_painter.h"
#include "kis_int_spinbox.h"
#include "KoPointerEvent.h"
#include "kis_paintop_registry.h"
#include "QPainter"
#include "kis_cursor.h"
#include "kis_int_spinbox.h"

#include "kis_tool_star.h"

KisToolStar::KisToolStar(KoCanvasBase * canvas)
    : super(canvas, KisCursor::load("tool_star_cursor.png", 6, 6)),
      m_dragging (false)
{
    setObjectName("tool_star");
    m_innerOuterRatio=40;
    m_vertices=5;
}

KisToolStar::~KisToolStar()
{
}
void KisToolStar::mousePressEvent(KoPointerEvent *event)
{
    if (m_canvas && event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = convertToPixelCoord(event);
        m_dragEnd = convertToPixelCoord(event);
        m_vertices = m_optWidget->verticesSpinBox->value();
        m_innerOuterRatio = m_optWidget->ratioSpinBox->value();
    }
}

void KisToolStar::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
        //Erase old lines
        m_canvas->updateCanvas(boundingRect());
        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = convertToPixelCoord(event) - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            m_dragEnd = convertToPixelCoord(event);
        }
        m_canvas->updateCanvas(boundingRect());
    }
}

void KisToolStar::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!m_canvas)
        return;

    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;

        if (m_dragStart == m_dragEnd)
            return;

        if (!m_currentImage)
            return;

        if (!m_currentImage->activeDevice())
            return;

        KisPaintDeviceSP device = m_currentImage->activeDevice ();
        KisPainter painter (device);
        painter.beginTransaction (i18n("Star"));

        painter.setPaintColor(m_currentFgColor);
        painter.setBackgroundColor(m_currentBgColor);
        painter.setFillStyle(fillStyle());
        painter.setBrush(m_currentBrush);
        painter.setPattern(m_currentPattern);
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_currentPaintOp, m_currentPaintOpSettings, &painter);
        painter.setPaintOp(op); // Painter takes ownership

        vQPointF coord = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());

        painter.paintPolygon(coord);

        device->setDirty( painter.dirtyRegion() );
        notifyModified();

        m_canvas->addCommand(painter.endTransaction());
    }
}

void KisToolStar::paint(QPainter& gc, KoViewConverter &converter)
{
    double sx, sy;
    converter.zoom(&sx, &sy);

    gc.scale( sx/m_currentImage->xRes(), sy/m_currentImage->yRes() );

    if (!m_canvas)
        return;

    QPen pen(Qt::SolidLine);
    gc.setPen(pen);

    vQPointF points = starCoordinates(m_vertices, m_dragStart.x(), m_dragStart.y(), m_dragEnd.x(), m_dragEnd.y());

    for (int i = 0; i < points.count() - 1; i++) {
        gc.drawLine(points[i], points[i + 1]);
    }
    gc.drawLine(points[points.count() - 1], points[0]);
}

vQPointF KisToolStar::starCoordinates(int N, double mx, double my, double x, double y)
{
    double R=0, r=0;
    qint32 n=0;
    double angle;

    vQPointF starCoordinatesArray(2*N);

    // the radius of the outer edges
    R=sqrt((x-mx)*(x-mx)+(y-my)*(y-my));

    // the radius of the inner edges
    r=R*m_innerOuterRatio/100.0;

    // the angle
    angle=-atan2((x-mx),(y-my));

    //set outer edges
    for(n=0;n<N;n++){
        starCoordinatesArray[2*n] = QPointF(mx+R*cos(n * 2.0 * M_PI / N + angle),my+R*sin(n *2.0 * M_PI / N+angle));
    }

    //set inner edges
    for(n=0;n<N;n++){
        starCoordinatesArray[2*n+1] = QPointF(mx+r*cos((n + 0.5) * 2.0 * M_PI / N + angle),my+r*sin((n +0.5) * 2.0 * M_PI / N + angle));
    }

    return starCoordinatesArray;
}

QRectF KisToolStar::boundingRect()
{
    //Calculating the radius
    double radius = sqrt((m_dragEnd.x()-m_dragStart.x())*(m_dragEnd.x()-m_dragStart.x())+(m_dragEnd.y()-m_dragStart.y())*((m_dragEnd.y()-m_dragStart.y())));
    return QRectF(m_dragStart.x()-radius, m_dragStart.y()-radius, 2*radius, 2*radius);
}

QWidget* KisToolStar::createOptionWidget()
{
    QWidget *widget = super::createOptionWidget();

    m_optWidget = new WdgToolStar(widget);
    Q_CHECK_PTR(m_optWidget);

    m_optWidget->ratioSpinBox->setValue(m_innerOuterRatio);

    QGridLayout *optionLayout = new QGridLayout(widget);
    super::addOptionWidgetLayout(optionLayout);

    optionLayout->addWidget(m_optWidget, 0, 0);

    return widget;
}

#include "kis_tool_star.moc"
