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

#include "spray_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QTransform>

#include "kis_random_accessor.h"
#include "kis_painter.h"

#include <cmath>

#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif

SprayBrush::SprayBrush(const KoColor &inkColor)
{
    m_inkColor = inkColor;
    m_counter = 0;
    srand48(time(0));
}

SprayBrush::SprayBrush()
{
    m_radius = 0;
    m_counter = 0;
}



void SprayBrush::paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color)
{
    // initializing painter
    KisPainter drawer(dev);
    drawer.setPaintColor(color);

    // jitter radius
    int tmpRadius = m_radius;
    if (m_jitterSize){
        m_radius = m_radius * drand48();
    }

    // jitter movement
    if (m_jitterMovement){
        x = x + (( 2 * m_radius * drand48() ) - m_radius) * m_amount;
        y = y + (( 2 * m_radius * drand48() ) - m_radius) * m_amount;
    }

    m_pixelSize = dev->colorSpace()->pixelSize();
    m_inkColor = color;
    m_counter++;

    qint32 pixelSize = dev->colorSpace()->pixelSize();
    KisRandomAccessor accessor = dev->createRandomAccessor((int)x, (int)y);

/* Not using now */
// random coloring
//            KoColorTransformation* transfo; QHash<QString, QVariant> params;
//         params["h"] = (360.0 * drand48()) - 180.0;
//         params["s"] = (200.0 * drand48()) - 100.0;
//         params["v"] = (200.0 * drand48()) - 100.0;;
//         transfo = dev->colorSpace()->createColorTransformation("hsv_adjustment", params);
//         transfo->transform(color.data(), m_inkColor.data(), 1);

    // coverage: adaptively select how many objects are sprayed per paint
    int points = (m_coverage * (M_PI * m_radius * m_radius) );

    int opacity = 255;
    m_inkColor.setOpacity(opacity);

    qreal nx, ny;
    int ix, iy;

    qreal angle;
    qreal lengthX;
    qreal lengthY;
    
    for (int i = 0; i < points; i++){
        // generate random angle
        angle = drand48() * M_PI * 2;
        // different X and Y length??
        lengthY = lengthX = drand48();
        // I hope we live the era where sin and cos is not slow for spray
        nx = (sin(angle) * m_radius * lengthX);
        ny = (cos(angle) * m_radius * lengthY);
        // transform
        nx *= m_scale;
        ny *= m_scale;

        // it is some shape (circle, ellipse, rectangle)
        if (m_object == 0)
        {
            // steps for single step in circle and ellipse
            int steps = 36;
            qreal random = drand48();       
            drawer.setFillColor(m_inkColor);
            drawer.setBackgroundColor(m_inkColor);
            // it is circle
            if (m_shape == 0){
                // (m_width == m_height) should be done in GUI somehow
                qreal circleRadius = m_width / 2.0;
                if (m_jitterShapeSize){
                    paintCircle(drawer, nx + x, ny + y, int((random * circleRadius) + 1.5) , steps);
                } else{
                    paintCircle(drawer, nx + x, ny + y, qRound(circleRadius)  , steps);
                }
                
            } else if (m_shape == 1){
                
                qreal ellipseA = m_width / 2.0;
                qreal ellipseB = m_height / 2.0;

                if (m_jitterShapeSize){
                    paintEllipse(drawer, nx + x, ny + y,int((random * ellipseA) + 1.5) ,int((random * ellipseB) + 1.5), angle , steps);
                } else{
                    paintEllipse(drawer, nx + x, ny + y, qRound(ellipseA), qRound(ellipseB), angle , steps);
                }

            } else if (m_shape == 2){
                if (m_jitterShapeSize){
                    paintRectangle(drawer, nx + x, ny + y,int((random * m_width) + 1.5) ,int((random * m_height) + 1.5), angle , steps);
                } else{
                    paintRectangle(drawer, nx + x, ny + y, qRound(m_width), qRound(m_height), angle , steps);
                }
            }    
        // it is particle
        }else if (m_object == 1){
            paintParticle(accessor,m_inkColor,nx + x, ny + y);
        }
        // it is pixel
        else if (m_object == 2)
        {
            ix = qRound(nx + x);
            iy = qRound(ny + y);
            accessor.moveTo(ix, iy);
            memcpy(accessor.rawData(), m_inkColor.data(), pixelSize);
        }
    }

    // recover from jittering of color
    m_radius = tmpRadius;
}

SprayBrush::~SprayBrush()
{
}


void SprayBrush::paintParticle(KisRandomAccessor &writeAccessor,const KoColor &color,qreal rx, qreal ry){
    qreal MAX_OPACITY = 255;
    // opacity top left, right, bottom left, right
    KoColor pcolor(color);

    int ipx = int ( rx );
    int ipy = int ( ry );   
    qreal fx = rx - ipx;
    qreal fy = ry - ipy;

    int btl = qRound( ( 1-fx ) * ( 1-fy ) * MAX_OPACITY );
    int btr = qRound( ( fx )  * ( 1-fy ) * MAX_OPACITY );
    int bbl = qRound( ( 1-fx ) * ( fy )  * MAX_OPACITY );
    int bbr = qRound( ( fx )  * ( fy )  * MAX_OPACITY );

    // this version overwrite pixels, e.g. when it sprays two particle next
    // to each other, the pixel with lower opacity can override other pixel.
    // Maybe some kind of compositing using here would be cool

    pcolor.setOpacity ( btl );
    writeAccessor.moveTo ( ipx  , ipy );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );

    pcolor.setOpacity ( btr );
    writeAccessor.moveTo ( ipx + 1, ipy );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );

    pcolor.setOpacity ( bbl );
    writeAccessor.moveTo ( ipx, ipy + 1 );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );

    pcolor.setOpacity ( bbr );
    writeAccessor.moveTo ( ipx + 1, ipy + 1 );
    memcpy ( writeAccessor.rawData(), pcolor.data(), m_pixelSize );
}

void SprayBrush::paintCircle(KisPainter& painter, qreal x, qreal y, int radius, int steps) {
    QVector<QPointF> points;
    // circle x, circle y
    qreal cx, cy;

    qreal length = 2.0 * M_PI;
    qreal step = 1.0 / steps;
    for (int i = 0; i < steps; i++){
        cx = cos(i * step * length);
        cy = sin(i * step * length);

        cx *= radius;
        cy *= radius;

        cx += x;
        cy += y;

        points.append( QPointF(cx, cy) );
    }
    painter.setOpacity( int( ( 255 * drand48() ) + 0.5 ) );
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.paintPolygon( points );
}



void SprayBrush::paintEllipse(KisPainter& painter, qreal x, qreal y, int a, int b, qreal angle, int steps) {
    QVector <QPointF> points;
    qreal beta = -angle;
    qreal sinbeta = sin(beta);
    qreal cosbeta = cos(beta);

    for (int i = 0; i < 360; i += 360.0 / steps)
    {
        qreal alpha = i * (M_PI / 180) ;
        qreal sinalpha = sin(alpha);
        qreal cosalpha = cos(alpha);
 
        qreal X = x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta);
        qreal Y = y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta);
 
        points.append( QPointF(X, Y) );
   }
    painter.setOpacity( int( ( 255 * drand48() ) + 0.5 ) );
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.paintPolygon( points );
}

void SprayBrush::paintRectangle(KisPainter& painter, qreal x, qreal y, int width, int height, qreal angle, int steps) {
    QVector <QPointF> points;
    QTransform transform;

    qreal halfWidth = width / 2.0;
    qreal halfHeight = height / 2.0;
    qreal tx, ty;

    

    transform.reset();
    transform.rotateRadians( angle );
    // top left
    transform.map( - halfWidth,  - halfHeight, &tx, &ty);
    points.append(QPointF(tx + x,ty + y));
    // top right
    transform.map( + halfWidth,  - halfHeight, &tx, &ty);
    points.append(QPointF(tx + x,ty + y));
    // bottom right
    transform.map( + halfWidth,  + halfHeight, &tx, &ty);
    points.append(QPointF(tx + x,ty + y));
    // botom left 
    transform.map( - halfWidth,  + halfHeight, &tx, &ty);
    points.append(QPointF(tx + x,ty + y));


    painter.setOpacity( int( ( 255 * drand48() ) + 0.5 ) );
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.paintPolygon( points );
}




