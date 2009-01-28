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

#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif


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

    if (!first){
        m_mouse.init(mx,my);
        m_odelx = 0.0;
        m_odely = 0.0;
        first = true;
        return;
    }

    KisPainter drawer(dev);
    drawer.setPaintColor(color);
    //drawer.drawThickLine(QPointF(x,y), QPointF(x,y)+QPointF(20,20), 5,10);

    if (applyFilter(mx, my)){
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

    if(m_mouse.acc < 0.000001){
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
    if(m_mouse.vel < 0.000001){
        return 0;
    }

    /* calculate angle of drawing tool */
    m_mouse.angx /= m_mouse.vel;
    m_mouse.angy /= m_mouse.vel;
    if(m_mouse.fixedangle) {
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

//  wid = 0.04 - m_mouse.vel;
    wid = m_widthRange - m_mouse.vel;

    wid = wid * m_width;

    if(wid < 0.00001){
        wid = 0.00001;
    }

    delx = m_mouse.angx * wid;
    dely = m_mouse.angy * wid;

    px = m_mouse.lastx;
    py = m_mouse.lasty;
    nx = m_mouse.curx;
    ny = m_mouse.cury;

    // TODO : kritadraw polygon here..
    qreal px1 = px + m_odelx;
    qreal py1 = py + m_odely;

    qreal px2 = px - m_odelx;
    qreal py2 = py - m_odely;

    qreal px3 = nx - delx;
    qreal py3 = ny - dely;

    qreal px4 = nx + delx;
    qreal py4 = ny + dely;

    px1 *= m_image->width();
    px2 *= m_image->width();
    px3 *= m_image->width();
    px4 *= m_image->width();

    py1 *= m_image->height();
    py2 *= m_image->height();
    py3 *= m_image->height();
    py4 *= m_image->height();

    QPointF p1(px1,py1 );
    QPointF p2(px2,py2 );
    QPointF p3(px3,py3 );
    QPointF p4(px4,py4 );


    QVector<QPointF> pole;
    pole.append(p1);
    pole.append(p2);
    pole.append(p3);
    pole.append(p4);

    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setStrokeStyle(KisPainter::StrokeStyleNone);
    painter.paintPolygon( pole );
    pole.clear();
//    painter.drawLine( p1, p2 );
//    painter.drawLine( p2, p3 );
//    painter.drawLine( p3, p4 );
//    painter.drawLine( p4, p1 );

    QPointF start( px * m_image->width(), py * m_image->height() );
    QPointF end( nx * m_image->width(), ny * m_image->height() );

    painter.drawLine( start, end );

    m_odelx = delx;
    m_odely = dely;
}


