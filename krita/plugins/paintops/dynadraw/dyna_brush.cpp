/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#include <kis_random_accessor.h>
#include <kis_painter.h>

#include <cmath>

#include <QVector>


DynaBrush::DynaBrush()
{
    first = false;
    m_counter = 0;

    /* dynadraw init */
    m_curmass = 0.5;
    m_curdrag = 0.15;
    m_mouse.fixedangle = true;
    m_width = 1.5;
    m_xangle = 0.60;
    m_yangle = 0.20;
    m_widthRange = 0.05;

}

void DynaBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    /*    KisRandomAccessor accessor = dev->createRandomAccessor((int)x, (int)y);
        m_pixelSize = dev->colorSpace()->pixelSize();
        m_inkColor = color;*/

    qreal mx, my;
    mx = m_mousePos.x();
    my = m_mousePos.y();

    if (!first) {
        m_mouse.init(mx, my);
        m_odelx = 0.0;
        m_odely = 0.0;

        for (int i = 0; i < m_circleRadius; i++) {
            m_prevPosition.append(QPointF(x, y));
        }

        first = true;
        return;
    }

    KisPainter drawer(dev);
    drawer.setPaintColor(color);
    //drawer.drawThickLine(QPointF(x,y), QPointF(x,y)+QPointF(20,20), 5,10);

    if (applyFilter(mx, my)) {
        drawSegment(drawer);
    }

    m_counter++;
}

DynaBrush::~DynaBrush()
{
}

int DynaBrush::applyFilter(qreal mx, qreal my)
{
    qreal mass, drag;
    qreal fx, fy;

    /* calculate mass and drag */
    mass = flerp(1.0, 160.0, m_curmass);
    drag = flerp(0.00, 0.5, m_curdrag * m_curdrag);

    /* calculate force and acceleration */
    fx = mx - m_mouse.curx;
    fy = my - m_mouse.cury;

    m_mouse.acc = sqrt(fx * fx + fy * fy);

    if (m_mouse.acc < 0.000001) {
        return 0;
    }

    m_mouse.accx = fx / mass;
    m_mouse.accy = fy / mass;

    /* calculate new velocity */
    m_mouse.velx += m_mouse.accx;
    m_mouse.vely += m_mouse.accy;
    m_mouse.vel = sqrt(m_mouse.velx * m_mouse.velx + m_mouse.vely * m_mouse.vely);
    m_mouse.angx = -m_mouse.vely;
    m_mouse.angy = m_mouse.velx;
    if (m_mouse.vel < 0.000001) {
        return 0;
    }

    /* calculate angle of drawing tool */
    m_mouse.angx /= m_mouse.vel;
    m_mouse.angy /= m_mouse.vel;
    if (m_mouse.fixedangle) {
        m_mouse.angx = m_xangle;
        m_mouse.angy = m_yangle;
    }

    m_mouse.velx = m_mouse.velx * (1.0 - drag);
    m_mouse.vely = m_mouse.vely * (1.0 - drag);

    m_mouse.lastx = m_mouse.curx;
    m_mouse.lasty = m_mouse.cury;
    m_mouse.curx = m_mouse.curx + m_mouse.velx;
    m_mouse.cury = m_mouse.cury + m_mouse.vely;

    return 1;
}


void DynaBrush::drawSegment(KisPainter &painter)
{
    qreal delx, dely;
    qreal wid;
    qreal px, py, nx, ny;

    wid = m_widthRange - m_mouse.vel;

    wid = wid * m_width;

    if (wid < 0.00001) {
        wid = 0.00001;
    }

    delx = m_mouse.angx * wid;
    dely = m_mouse.angy * wid;

    px = m_mouse.lastx;
    py = m_mouse.lasty;
    nx = m_mouse.curx;
    ny = m_mouse.cury;

    QPointF prev(px , py);         // previous position
    QPointF now(nx , ny);           // new position

    QPointF prevr(px + m_odelx , py + m_odely);
    QPointF prevl(px - m_odelx , py - m_odely);

    QPointF nowl(nx - delx , ny - dely);
    QPointF nowr(nx + delx , ny + dely);

    // transform coords from float points into image points
    prev.rx() *= m_image->width();
    prevr.rx() *= m_image->width();
    prevl.rx() *= m_image->width();
    now.rx()  *= m_image->width();
    nowl.rx() *= m_image->width();
    nowr.rx() *= m_image->width();

    prev.ry() *= m_image->height();
    prevr.ry() *= m_image->height();
    prevl.ry() *= m_image->height();
    now.ry()  *= m_image->height();
    nowl.ry() *= m_image->height();
    nowr.ry() *= m_image->height();

    if (m_enableLine)
        painter.drawLine(prev, now);

    if (m_action == 0) {
        qreal screenX = m_mouse.velx * m_image->width();
        qreal screenY = m_mouse.vely * m_image->height();
        qreal speed = sqrt(screenX * screenX + screenY * screenY);
        speed = qBound(0.0, speed , m_circleRadius * 2.0);

        drawCircle(painter, prev.x(), prev.y() , m_circleRadius + speed, 2 * m_circleRadius  + speed);
        //painter.paintEllipse(prevl.x(), prevl.y(), qAbs((prevl - prevr).x()), qAbs((prevl - prevr).y()) );
        if (m_twoCircles) {
            drawCircle(painter, now.x(), now.y() , m_circleRadius + speed, 2 * m_circleRadius + speed);
            //drawCircle(painter, now.x(), now.y() , m_circleRadius * m_mouse.vel , 2 * m_circleRadius * m_mouse.vel );
            //  painter.paintEllipse(nowl.x(), nowl.y(), qAbs((nowl - nowr).x()), qAbs((nowl - nowr).y()) );
        }
    } else if (m_action == 1) {
        drawQuad(painter, prevr, prevl, nowl, nowr);
    } else if (m_action == 2) {
        drawWire(painter, prevr, prevl, nowl, nowr);
    } else if (m_action == 3) {
        drawLines(painter, prev, now, m_lineCount);
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
    QPointF p1, p2;
    qreal offsetX, offsetY;

    int half = count / 2;
    for (int i = 0; i < count; i++) {
        offsetX = m_mouse.angx * (i - half) * m_lineSpacing * m_mouse.acc;
        offsetY = m_mouse.angy * (i - half) * m_lineSpacing * m_mouse.acc;

        p2.setX(now.x() + offsetX);
        p2.setY(now.y() + offsetY);

        painter.drawLine(m_prevPosition[i], p2);
        m_prevPosition[i] = p2;
    }
}
