/*
 *
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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
#include <QLayout>
#include <QWidget>
#include <QLabel>

#include <kdebug.h>
#include <klocale.h>

#include "kis_tool_measure.h"
#include "kis_image.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"

#define INNER_RADIUS 50

KisToolMeasure::KisToolMeasure(KoCanvasBase * canvas)
    : KisTool(canvas, QCursor(Qt::CrossCursor)),
      m_dragging( false )
{
    m_currentImage = 0;
    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);
}

KisToolMeasure::~KisToolMeasure()
{
}

void KisToolMeasure::paint(QPainter& gc, KoViewConverter &converter)
{
    double sx, sy;
    converter.zoom(&sx, &sy);

    gc.scale( sx/m_currentImage->xRes(), sy/m_currentImage->yRes() );

    QPen old = gc.pen();
    QPen pen(Qt::SolidLine);
    QPointF start;
    QPointF end;

    start = m_startPos;
    end = m_endPos;

    gc.setPen(pen);
    start = QPoint(static_cast<int>(start.x()), static_cast<int>(start.y()));
    end = QPoint(static_cast<int>(end.x()), static_cast<int>(end.y()));
    gc.drawLine(start, end);

    if(deltaX() >= 0)
        gc.drawLine(start.x(), start.y(), start.x()+INNER_RADIUS, start.y());
    else
        gc.drawLine(start.x(), start.y(), start.x()-INNER_RADIUS, start.y());

    if(distance() >= INNER_RADIUS){
        QRectF rectangle(start.x()-INNER_RADIUS, start.y()-INNER_RADIUS, 2*INNER_RADIUS, 2*INNER_RADIUS);
        int startAngle = (deltaX() >= 0) ? 0 : 180 * 16;

        int spanAngle;
        if((deltaY() >= 0 && deltaX() >= 0) || (deltaY() < 0 && deltaX() < 0))
            spanAngle = angle() * 16;
        else
            spanAngle = -angle() * 16;
        gc.drawArc(rectangle, startAngle, spanAngle);
    }

    gc.setPen(old);
}

void KisToolMeasure::mousePressEvent(KoPointerEvent *e)
{
    if (!m_currentImage) return;

    // Erase old temporary lines
    m_canvas->updateCanvas(convertToPt(boundingRect()));

    QPointF pos = convertToPixelCoord(e);

    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_startPos = pos;
        m_endPos = pos;
    }
    emit sigDistanceChanged(0);
    emit sigAngleChanged(0);
}

void KisToolMeasure::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_dragging) {
        // Erase old temporary lines
        m_canvas->updateCanvas(convertToPt(boundingRect()));

        QPointF pos = convertToPixelCoord(e);

        if (e->modifiers() & Qt::AltModifier) {
            QPointF trans = pos - m_endPos;
            m_startPos += trans;
            m_endPos += trans;
        }
        else
            m_endPos = pos;

        m_canvas->updateCanvas(convertToPt(boundingRect()));
    }
    emit sigDistanceChanged(distance());
    emit sigAngleChanged(angle());
}

void KisToolMeasure::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_dragging && e->button() == Qt::LeftButton) {
        m_dragging = false;
    }
}

QWidget* KisToolMeasure::createOptionWidget()
{
    m_optWidget = new QWidget();
    Q_CHECK_PTR(m_optWidget);

    QLabel* distanceLabel = new QLabel(i18n("Distance: "), m_optWidget);
    QLabel* angleLabel = new QLabel(i18n("Angle: "), m_optWidget);

    QLabel* distance = new QLabel(i18n("1234 "), m_optWidget);
    QLabel* angle = new QLabel(i18n("5678"), m_optWidget);
    connect(this, SIGNAL(sigDistanceChanged(double)), distance, SLOT(setNum(double)));
    connect(this, SIGNAL(sigAngleChanged(double)), angle, SLOT(setNum(double)));

    QGridLayout* optionLayout = new QGridLayout(m_optWidget);
    Q_CHECK_PTR(optionLayout);

    optionLayout->setMargin(0);
    optionLayout->setSpacing(6);

    optionLayout->addWidget(distanceLabel, 0, 0);
    optionLayout->addWidget(angleLabel, 1, 0);
    optionLayout->addWidget(distance, 0, 1);
    optionLayout->addWidget(angle, 1, 1);

    return m_optWidget;
}

double KisToolMeasure::angle()
{
    return atan(qAbs(deltaY())/qAbs(deltaX()))/(2*M_PI)*360;
}

double KisToolMeasure::distance()
{
    return sqrt(deltaX()*deltaX()+deltaY()*deltaY());
}

QRectF KisToolMeasure::boundingRect()
{
    QRectF bound;
    bound.setTopLeft(m_startPos);
    bound.setBottomRight(m_endPos);
    bound = bound.united(QRectF(m_startPos.x()-INNER_RADIUS, m_startPos.y()-INNER_RADIUS, 2*INNER_RADIUS, 2*INNER_RADIUS));
    return bound.normalized();
}

#include "kis_tool_measure.moc"

