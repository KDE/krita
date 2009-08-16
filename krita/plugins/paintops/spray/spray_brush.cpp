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
#include "kis_paint_information.h"

#include "kis_spray_paintop_settings.h"

#include "metaball.h"

#include <cmath>

// this is wrong drand48 gives 0.0 - 1.0, rand on WIN32 gives MAX_RAND 
#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif

#include "random_gauss.h"

SprayBrush::SprayBrush()
{
    m_radius = 0;
    m_counter = 0;
    m_randomOpacity = false;

    srand48( time(0) );
    m_rand = new RandomGauss( time(0) );
}

SprayBrush::~SprayBrush()
{
    delete m_rand;
}


void SprayBrush::paint(KisPaintDeviceSP dev, const KisPaintInformation& info, const KoColor &color)
{
    
    qreal x = info.pos().x();
    qreal y = info.pos().y();

    // initializing painter
    KisPainter drawer(dev);
    KisRandomAccessor accessor = dev->createRandomAccessor( qRound(x), qRound(y) );
    m_pixelSize = dev->colorSpace()->pixelSize();
    m_inkColor = color;

if (m_settings->useRandomHSV()){
    QHash<QString, QVariant> params;
    params["h"] = (m_settings->hue() / 180.0) * drand48();
    params["s"] = (m_settings->saturation() / 100.0) * drand48();
    params["v"] = (m_settings->value() / 100.0) * drand48();

    KoColorTransformation* transfo;
    transfo = dev->colorSpace()->createColorTransformation("hsv_adjustment", params);
    transfo->transform(color.data(), m_inkColor.data() , 1);
}

    drawer.setPaintColor(m_inkColor);
    m_counter++;

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

    // coverage: adaptively select how many objects are sprayed per paint
    if (m_useDensity){
        m_particlesCount = (m_coverage * (M_PI * m_radius * m_radius) );
    }

    // Metaballs are rendered little differently
    if (m_shape == 2 && m_object == 0){
        paintMetaballs(dev, info, color);
    }

    qreal nx, ny;
    int ix, iy;

    qreal angle;
    qreal lengthX;
    qreal lengthY;
    
    
    for (int i = 0; i < m_particlesCount; i++){
        // generate random angle
        angle = drand48() * M_PI * 2;

        if (m_settings->gaussian()){
            lengthY = lengthX = qBound(0.0, m_rand->nextGaussian(0.0, 0.50) , 1.0 );
        }else{
            lengthY = lengthX = drand48();
        }
        
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

            drawer.setPaintColor(m_inkColor);
            // it is ellipse
            if (m_shape == 0){
                //  
                qreal ellipseA = m_width / 2.0;
                qreal ellipseB = m_height / 2.0;

                if (m_width == m_height)
                {
                    if (m_jitterShapeSize){
                        paintCircle(drawer, nx + x, ny + y, int((random * ellipseA) + 1.5) , steps);
                    } else{
                        paintCircle(drawer, nx + x, ny + y, qRound(ellipseA)  , steps);
                    }
                } else 
                {
                    if (m_jitterShapeSize){
                        paintEllipse(drawer, nx + x, ny + y,int((random * ellipseA) + 1.5) ,int((random * ellipseB) + 1.5), angle , steps);
                    } else{
                        paintEllipse(drawer, nx + x, ny + y, qRound(ellipseA), qRound(ellipseB), angle , steps);
                    }
                }
            } else if (m_shape == 1)
            {
                if (m_jitterShapeSize){
                    paintRectangle(drawer, nx + x, ny + y,int((random * m_width) + 1.5) ,int((random * m_height) + 1.5), angle , steps);
                } else{
                    paintRectangle(drawer, nx + x, ny + y, qRound(m_width), qRound(m_height), angle , steps);
                }
            }    
        // it is pixel particle
        }else if (m_object == 1){
            if (m_randomOpacity)
            {
                m_inkColor.setOpacity( OPACITY_OPAQUE * drand48() );
            }
            paintParticle(accessor,m_inkColor,nx + x, ny + y);
        }
        // it is pixel
        else if (m_object == 2)
        {
            ix = qRound(nx + x);
            iy = qRound(ny + y);
            accessor.moveTo(ix, iy);
            if (m_randomOpacity)
            {
                m_inkColor.setOpacity( OPACITY_OPAQUE * drand48() );
            }
            memcpy(accessor.rawData(), m_inkColor.data(), m_pixelSize);
        }
    }

    

    // hidden code for outline detection
    //m_inkColor.setOpacity(128);
    //paintOutline(dev,m_inkColor,x, y, m_radius * 2);

    // recover from jittering of color,
    // m_inkColor.opacity is recovered with every paint
    m_radius = tmpRadius;
    
}



void SprayBrush::paintParticle(KisRandomAccessor &writeAccessor,const KoColor &color,qreal rx, qreal ry){
    // opacity top left, right, bottom left, right
    KoColor pcolor(color);
    int opacity = pcolor.opacity();

    int ipx = int ( rx );
    int ipy = int ( ry );   
    qreal fx = rx - ipx;
    qreal fy = ry - ipy;

    int btl = qRound( ( 1-fx ) * ( 1-fy ) * opacity );
    int btr = qRound( ( fx )  * ( 1-fy ) * opacity );
    int bbl = qRound( ( 1-fx ) * ( fy )  * opacity );
    int bbr = qRound( ( fx )  * ( fy )  * opacity );

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

    if (m_randomOpacity)
    {
        painter.setOpacity( qRound(  OPACITY_OPAQUE * drand48()  ) );
    }

    painter.setFillStyle( KisPainter::FillStyleForegroundColor );
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

    if (m_randomOpacity)
    {
        painter.setOpacity( int( ( OPACITY_OPAQUE * drand48() ) + 0.5 ) );
    }

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

    if (m_randomOpacity)
    {
        painter.setOpacity( int( ( OPACITY_OPAQUE * drand48() ) + 0.5 ) );
    }

    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.paintPolygon( points );
}

void SprayBrush::paintDistanceMap(KisPaintDeviceSP dev, const KisPaintInformation &info, const KoColor &painterColor){
    KisRandomAccessor accessor = dev->createRandomAccessor(0, 0);
    KoColor color = painterColor;

    qreal posX = info.pos().x();
    qreal posY = info.pos().y();
    
    qreal opacity = 255;
    for (int y = -m_radius; y <= m_radius; y++){
        for (int x = -m_radius; x <= m_radius; x++){
            //opacity = sqrt(y*y + x*x) / m_radius;
            opacity = (y*y + x*x) / (m_radius * m_radius);
            opacity = 1.0 - opacity;
            opacity *= m_scale;

            if (opacity < 0) continue;
            if (opacity > 1.0) opacity = 1.0;

            if ( (y*y + x*x) <= (m_radius * m_radius) )
            {
                color.setOpacity( opacity * 255);
                accessor.moveTo(x + posX, y + posY);
                memcpy( accessor.rawData(), color.data(), dev->colorSpace()->pixelSize() );
            }

        }
    }
}

void SprayBrush::paintMetaballs(KisPaintDeviceSP dev, const KisPaintInformation &info, const KoColor &painterColor) {
    qreal MIN_TRESHOLD = m_mintresh;
    qreal MAX_TRESHOLD = m_maxtresh;

    KoColor color = painterColor;
    qreal posX = info.pos().x();
    qreal posY = info.pos().y();

    //int points = m_coverage * (m_radius * m_radius * M_PI);
    qreal ballRadius = m_width * 0.5;

    // generate metaballs
    QList<Metaball> list;
    for (int i = 0; i < m_particlesCount ; i++){
        qreal x = (2 * drand48() * m_radius) - m_radius;
        qreal y = (2 * drand48() * m_radius) - m_radius;
        list.append(
                    Metaball( x, 
                              y ,
                              drand48() *  ballRadius)
                    );
    }

 
    // paint it
    KisRandomAccessor accessor = dev->createRandomAccessor(0, 0);

    qreal sum = 0.0;
    m_computeArea.translate( -qRound(posX), -qRound(posY) );
    for (int y = m_computeArea.y(); y <= m_computeArea.height(); y++){
        for (int x = m_computeArea.x() ; x <= m_computeArea.width(); x++){

            sum = 0.0;        

            for (int i = 0; i < m_particlesCount; i++){
                sum += list[i].equation(x, y );
            }
           
            if (sum >= MIN_TRESHOLD && sum <= MAX_TRESHOLD){
                    if (sum < 0.0) sum = 0.0;
                    if (sum > 1.0) sum = 1.0;

                    color.setOpacity(OPACITY_OPAQUE * sum);
                    accessor.moveTo( x + posX ,y + posY );
                    memcpy(accessor.rawData(), color.data(), dev->colorSpace()->pixelSize() );
            }
        }
    }
    m_computeArea.translate( qRound(posX), qRound(posY) );

#if 0        
        KisPainter dabPainter(dev);
        dabPainter.setFillColor(color);
        dabPainter.setPaintColor(color);
        dabPainter.setFillStyle(KisPainter::FillStyleForegroundColor);

        for (int i = 0; i < m_particlesCount; i++){
                qreal x = list[i].x() + posX;
                qreal y = list[i].y() + posY;
                dabPainter.paintEllipse(x, y, list[i].radius() * 2,list[i].radius() * 2);
        }
#endif

}


void SprayBrush::paintOutline(KisPaintDeviceSP dev ,const KoColor &outlineColor,qreal posX, qreal posY, qreal radius) {
    QList<QPointF> antiPixels;
    KisRandomAccessor accessor = dev->createRandomAccessor( qRound(posX), qRound(posY) );

    for (int y = -radius+posY; y <= radius+posY; y++){
        for (int x = -radius+posX; x <= radius+posX; x++){
            accessor.moveTo(x,y);
            qreal alpha = dev->colorSpace()->alpha(accessor.rawData());

            if (alpha != 0){
                // top left
                accessor.moveTo(x - 1,y - 1);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x - 1,y - 1) );
                    //continue;
                }

                // top
                accessor.moveTo(x,y - 1);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x,y - 1) );
                    //continue;
                }

                // top right
                accessor.moveTo(x + 1,y - 1);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x + 1,y - 1) );
                    //continue;
                }

                //left 
                accessor.moveTo(x - 1,y);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x - 1,y) );
                    //continue;
                }

                //right
                accessor.moveTo(x + 1,y);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x + 1,y) );
                    //continue;
                }

                // bottom left
                accessor.moveTo(x - 1,y + 1);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x - 1,y + 1) );
                    //continue;
                }

                // bottom
                accessor.moveTo(x,y + 1);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x,y + 1) );
                    //continue;
                }

                // bottom right
                accessor.moveTo(x + 1,y + 1);
                if ( dev->colorSpace()->alpha(accessor.rawData()) == 0){
                    antiPixels.append( QPointF(x + 1,y + 1) );
                    //continue;
                }
            }

        }
    }

    // anti-alias it
    int size = antiPixels.size();
    for (int i = 0; i < size; i++)
    {
        accessor.moveTo( antiPixels[i].x(), antiPixels[i].y() );
        memcpy(accessor.rawData(), outlineColor.data(), dev->colorSpace()->pixelSize() );
    }
}
