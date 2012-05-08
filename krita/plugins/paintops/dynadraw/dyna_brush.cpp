/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "dyna_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_random_accessor_ng.h>
#include <kis_painter.h>

#include <cmath>

#include <QVector>


DynaBrush::DynaBrush()
{
    m_initialized = false;
    m_counter = 0;

    // default values from Paul Haeberli code
    /*m_cursorFilter.setUseFixedAngle(true);
    m_cursorFilter.setFixedAngles(0.6,0.2); 
    initWidth = 1.5;
    widthRange = 0.05;*/
    m_odelx = 0.0;
    m_odely = 0.0;
}




void DynaBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    qreal mx = m_cursorPos.x();
    qreal my = m_cursorPos.y();

    if (!m_initialized) {
        m_cursorFilter.initFilterPosition(mx, my);
        m_cursorFilter.setUseFixedAngle(m_properties->useFixedAngle);
        m_cursorFilter.setFixedAngles(m_properties->xAngle,m_properties->yAngle);
        m_cursorFilter.setMass(m_properties->mass);
        m_cursorFilter.setDrag(m_properties->drag);

        for (quint16 i = 0; i < m_properties->lineCount; i++) {
            m_prevPosition.append(QPointF(x, y));
        }

        m_initialized = true;
        return;
    }

    KisPainter drawer(dev);
    drawer.setPaintColor(color);

    if (m_cursorFilter.applyFilter(mx, my)) {
        drawSegment(drawer);
    }
    m_counter++;
}

DynaBrush::~DynaBrush()
{
}

void DynaBrush::drawSegment(KisPainter &painter)
{
    qreal wid = (m_properties->widthRange - m_cursorFilter.velocity()) * m_properties->initWidth;
    
    if (wid < 0.00001) {
        wid = 0.00001;
    }

    qreal delx = m_cursorFilter.angleX() * wid;
    qreal dely = m_cursorFilter.angleY() * wid;
    
    qreal px = m_cursorFilter.prevX();
    qreal py = m_cursorFilter.prevY();
    qreal nx = m_cursorFilter.x();
    qreal ny = m_cursorFilter.y();

    QPointF prev(px , py);         // previous position
    QPointF now(nx , ny);           // new position

    QPointF prevr(px + m_odelx , py + m_odely);
    QPointF prevl(px - m_odelx , py - m_odely);

    QPointF nowl(nx - delx , ny - dely);
    QPointF nowr(nx + delx , ny + dely);

    // transform coords from float points into image points
    prev.rx() *= m_canvasWidth;
    prevr.rx() *= m_canvasWidth;
    prevl.rx() *= m_canvasWidth;
    now.rx()  *= m_canvasWidth;
    nowl.rx() *= m_canvasWidth;
    nowr.rx() *= m_canvasWidth;

    prev.ry() *= m_canvasHeight;
    prevr.ry() *= m_canvasHeight;
    prevl.ry() *= m_canvasHeight;
    now.ry()  *= m_canvasHeight;
    nowl.ry() *= m_canvasHeight;
    nowr.ry() *= m_canvasHeight;

    if (m_properties->enableLine){
        painter.drawLine(prev, now);
    }

    if (m_properties->action == 0) {
        qreal screenX = m_cursorFilter.velocityX() * m_canvasWidth;
        qreal screenY = m_cursorFilter.velocityY() * m_canvasHeight;
        qreal speed = sqrt(screenX * screenX + screenY * screenY);
        speed = qBound(qreal(0.0), speed , qreal(m_properties->diameter));

        drawCircle(painter, prev.x(), prev.y() , m_properties->diameter * 0.5 + speed, m_properties->diameter + speed);
        if (m_properties->useTwoCircles) {
            drawCircle(painter, now.x(), now.y() , m_properties->diameter * 0.5 + speed, m_properties->diameter + speed);
        }
        
    } else if (m_properties->action == 1) {
        drawQuad(painter, prevr, prevl, nowl, nowr);
    } else if (m_properties->action == 2) {
        drawWire(painter, prevr, prevl, nowl, nowr);
    } else if (m_properties->action == 3) {
        drawLines(painter, prev, now, m_properties->lineCount);
    }

    m_odelx = delx;
    m_odely = dely;
}


void DynaBrush::drawQuad(KisPainter &painter,
                         QPointF &topRight,
                         QPointF &topLeft,
                         QPointF &bottomLeft,
                         QPointF &bottomRight)
{
    QVector<QPointF> points;
    points.append(topRight);
    points.append(topLeft);
    points.append(bottomLeft);
    points.append(bottomRight);

    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.paintPolygon(points);
}

void DynaBrush::drawCircle(KisPainter &painter, qreal x, qreal y, int radius, int steps)
{
    QVector<QPointF> points;
    // circle x, circle y
    qreal cx, cy;

    qreal length = 2.0 * M_PI;
    qreal step = 1.0 / steps;
    for (int i = 0; i < steps; i++) {
        cx = cos(i * step * length);
        cy = sin(i * step * length);

        cx *= radius;
        cy *= radius;

        cx += x;
        cy += y;

        points.append(QPointF(cx, cy));
    }

    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.paintPolygon(points);
}

void DynaBrush::drawWire(KisPainter &painter,
                         QPointF &topRight,
                         QPointF &topLeft,
                         QPointF &bottomLeft,
                         QPointF &bottomRight)
{
    painter.drawLine(topRight, topLeft);
    painter.drawLine(topLeft, bottomLeft);
    painter.drawLine(bottomLeft, bottomRight);
    painter.drawLine(bottomRight, topRight);
}

void DynaBrush::drawLines(KisPainter &painter,
                          QPointF &prev,
                          QPointF &now,
                          int count
                         )
{
    Q_UNUSED(prev);
    QPointF p1, p2;
    qreal offsetX, offsetY;

    int half = count / 2;
    for (int i = 0; i < count; i++) {
        offsetX = m_cursorFilter.angleX() * (i - half) * m_properties->lineSpacing * m_cursorFilter.acceleration();
        offsetY = m_cursorFilter.angleY() * (i - half) * m_properties->lineSpacing * m_cursorFilter.acceleration();

        p2.setX(now.x() + offsetX);
        p2.setY(now.y() + offsetY);

        painter.drawLine(m_prevPosition[i], p2);
        m_prevPosition[i] = p2;
    }
}
