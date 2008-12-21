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

#include "curve_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include <kis_iterator.h>
#include <kis_random_accessor.h>
#include <kis_random_sub_accessor.h>

#include <cmath>
#include <ctime>
#include <QTime>

const qreal radToDeg = 57.29578;
const int POINTS = 4;
#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif

CurveBrush::CurveBrush() : m_head( 0 ), m_counter ( 0 ),m_incr ( 1 )
{
    for (int i=0;i<POINTS;i++){
        m_points[i] = QPointF(0,0);
    }

}

CurveBrush::~CurveBrush()
{
}

void CurveBrush::paint ( KisPaintDeviceSP dab,KisPaintDeviceSP layer, const KisPaintInformation &info )
{
    qreal x1 = info.pos().x();
    qreal y1 = info.pos().y();

    m_layer = layer;
    m_dab = dab;
    m_pixelSize = dab->colorSpace()->pixelSize();

    KisRandomAccessor accessor = dab->createRandomAccessor ( ( int ) x1, ( int ) y1 );
    m_writeAccessor = &accessor;

    KisRandomAccessor accessor2 = layer->createRandomAccessor ( ( int ) x1, ( int ) y1 );
    m_readAccessor = &accessor2;

    qreal length = 60.0;

    qreal p1x = x1-20;
    qreal p1y = y1+m_counter;

    qreal p2x = x1;
    qreal p2y = y1;

    qreal p3x = x1+length;
    qreal p3y = y1;

    qreal p4x = p3x+20;
    qreal p4y = p3y+m_counter;


    QPointF p0 ( p1x, p1y );
    QPointF p1 ( p2x, p2y );
    QPointF p2 ( p3x, p3y );
    QPointF p3 ( p4x, p4y );

    normalizePoints ( p0,p1,p2,p3 );

    int steps = 200;

    QPointF result;
    int paintX, paintY;
    int moveX, moveY;
    moveX = info.pos().x();
    moveY = info.pos().y();

    for ( int i = 0 ; i <= steps; i++ )
    {
        result = getCubicBezier ( p0,p1,p2,p3, ( qreal ) i/ ( qreal ) steps );
//         dbgPlugins << "x: " << result.x();
//         dbgPlugins << "y: " << result.y();
        paintX = int ( result.x() * m_image->width() + 0.5 );
        paintY = int ( result.y() * m_image->height() + 0.5 );
//         m_painter->drawLine(QPointF(moveX, moveY), QPointF(paintX, paintY) );
//         moveX = paintX;
//         moveY = paintY;
        dbgPlugins << "angle: " << info.angle();
        rotatePoints ( &paintX, &paintY,info.pos().x(),info.pos().y(), info.angle() +1.57 );
        m_writeAccessor->moveTo ( paintX  , paintY );
        memcpy ( m_writeAccessor->rawData() , m_inkColor.data() , m_pixelSize );
    }

    m_counter += m_incr;
    if ( abs ( m_counter )-1 == 30 )
    {
        m_incr *= -1;
    }

}

void CurveBrush::rotatePoints ( int *x, int *y, qreal centerX, qreal centerY,qreal angle )
{
    qreal mx = *x - centerX;
    qreal my = *y - centerY;

    qreal rotX = sin ( angle ) *mx + cos ( angle ) *my;
    qreal rotY = cos ( angle ) *mx + sin ( angle ) *my;

    rotX += centerX;
    rotY += centerY;
    *x = int ( rotX+0.5 );
    *y = int ( rotY+0.5 );

}

void CurveBrush::normalizePoints ( QPointF &p0, QPointF &p1, QPointF &p2, QPointF &p3 )
{
    p0.setX ( p0.x() / m_image->width() );
    p1.setX ( p1.x() / m_image->width() );
    p2.setX ( p2.x() / m_image->width() );
    p3.setX ( p3.x() / m_image->width() );

    p0.setY ( p0.y() / m_image->height() );
    p1.setY ( p1.y() / m_image->height() );
    p2.setY ( p2.y() / m_image->height() );
    p3.setY ( p3.y() / m_image->height() );

}

QPointF CurveBrush::getLinearPoint ( const QPointF &p1, const QPointF &p2,qreal u )
{
    qreal rx = ( 1.0 - u ) *p1.x() + u*p2.x();
    qreal ry = ( 1.0 - u ) *p1.y() + u*p2.y();
    return QPointF ( rx,ry );
}

QPointF CurveBrush::getBezierPoint ( QPointF &p0, QPointF &p1, QPointF &p2, QPointF &p3, qreal u )
{
    qreal rx =
        ( 1.0 - u ) * ( 1.0 - u ) * ( 1.0 - u ) * p0.x() +
        ( 1.0/3.0 ) * u * ( 1.0 - u ) * ( 1.0 - u ) * p1.x() +
        ( 1.0/3.0 ) * u * u * ( 1.0 - u ) * p2.x() +
        u * u * u * p3.x();



    qreal ry =
        ( 1.0 - u ) * ( 1.0 - u ) * ( 1.0 - u ) * p0.y() +
        ( 1.0/3.0 ) * u * ( 1.0 - u ) * ( 1.0 - u ) * p1.y() +
        ( 1.0/3.0 ) * u * u * ( 1.0 - u ) * p2.y() +
        u * u * u * p3.y();
    return QPointF ( rx,ry );
}

QPointF CurveBrush::getCubicBezier ( QPointF &p0, QPointF &p1, QPointF &p2, QPointF &p3, qreal u )
{
    qreal rx = pow ( u,3 ) * ( p3.x() + 3 * ( p1.x() - p2.x() ) - p0.x() )
               +3 * pow ( u,2 ) * ( p0.x() - 2*p1.x() + p2.x() )
               +3* u * ( p1.x() - p0.x() ) + p0.x();
    qreal ry = pow ( u,3 ) * ( p3.y() + 3 * ( p1.y() - p2.y() ) - p0.y() )
               +3 * pow ( u,2 ) * ( p0.y() - 2*p1.y() + p2.y() )
               +3* u * ( p1.y() - p0.y() ) + p0.y();

    return QPointF ( rx,ry );
}


void CurveBrush::paintLine ( KisPaintDeviceSP dab,KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2 )
{
    // Initialization
    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;
    qreal angle = atan2 ( dy, dx );
    qreal distance = sqrt ( dx * dx + dy * dy );

    KisRandomAccessor accessor = dab->createRandomAccessor ( ( int ) x1, ( int ) y1 );
    m_writeAccessor = &accessor;

    KisRandomAccessor accessor2 = layer->createRandomAccessor ( ( int ) x1, ( int ) y1 );
    m_readAccessor = &accessor2;

    m_layer = layer;
    m_dab = dab;
    m_pixelSize = dab->colorSpace()->pixelSize();

    int steps = 200;
    KoColor pcolor = m_inkColor;
    // end of initialization


    if (m_counter<4){
        m_painter->drawLine(pi1.pos() , pi2.pos() );
        addPoint( pi1.pos() );
    }else{

        addPoint( pi1.pos() );
        QPointF p0 = m_points[0];
        QPointF p1 = m_points[1];
        QPointF p2 = m_points[2];
        QPointF p3 = m_points[3];

        // debug control points

        QPointF result;
    
        normalizePoints ( p0,p1,p2,p3 );
        
        qreal paintX, paintY;
        int moveX, moveY;
    
        for ( int i = 0 ; i <= steps; i++ )
        {
            result = getCubicBezier ( p0,p1,p2,p3, ( qreal ) i/ ( qreal ) steps );
            paintX =  result.x() * m_image->width();
            paintY =  result.y() * m_image->height();
        
            /*
                moveX = paintX;
                moveY = paintY;
                rotatePoints ( &paintX, &paintY,pi1.pos().x(),pi1.pos().y(), pi1.angle() +1.57 );*/
        
            int ipx = int ( paintX );
            int ipy = int ( paintY );
            qreal fx = paintX - ipx;
            qreal fy = paintY - ipy;
        
            qreal MAX_OPACITY = 255;
        
            int btl = ( 1-fx ) * ( 1-fy ) * MAX_OPACITY;
            int btr = ( fx )  * ( 1-fy ) * MAX_OPACITY;
            int bbl = ( 1-fx ) * ( fy )  * MAX_OPACITY;
            int bbr = ( fx )  * ( fy )  * MAX_OPACITY;
        
            pcolor.setOpacity ( btl );
            m_writeAccessor->moveTo ( ipx  , ipy );
            if ( m_layer->colorSpace()->alpha ( m_writeAccessor->rawData() ) < pcolor.opacity() )
            {
                memcpy ( m_writeAccessor->rawData(), pcolor.data(), m_pixelSize );
            }
        
            pcolor.setOpacity ( btr );
            m_writeAccessor->moveTo ( ipx + 1, ipy );
            if ( m_layer->colorSpace()->alpha ( m_writeAccessor->rawData() ) < pcolor.opacity() )
            {
                memcpy ( m_writeAccessor->rawData(), pcolor.data(), m_pixelSize );
            }
        
            pcolor.setOpacity ( bbl );
            m_writeAccessor->moveTo ( ipx, ipy + 1 );
            if ( m_layer->colorSpace()->alpha ( m_writeAccessor->rawData() ) < pcolor.opacity() )
            {
                memcpy ( m_writeAccessor->rawData(), pcolor.data(), m_pixelSize );
            }
        
            pcolor.setOpacity ( bbr );
            m_writeAccessor->moveTo ( ipx + 1, ipy + 1 );
            if ( m_layer->colorSpace()->alpha ( m_writeAccessor->rawData() ) < pcolor.opacity() )
            {
                memcpy ( m_writeAccessor->rawData(), pcolor.data(), m_pixelSize );
            }
        
        }
        
    }
    dbgPlugins << m_counter;
    m_counter++;
}


void CurveBrush::addPoint(QPointF p){
    if ((m_head) == POINTS){
        removeLast();
    }
    m_points[m_head] = p; 
    m_head++;
}

void CurveBrush::removeLast(){
    for (int i = 1;i<POINTS; i++){
        m_points[i-1] = m_points[i];
    }
    m_head--;
}


void CurveBrush::debugColor ( const quint8* data )
{
    QColor rgbcolor;
    m_dab->colorSpace()->toQColor ( data, &rgbcolor );
    dbgPlugins << "RGBA: ("
    << rgbcolor.red()
    << ", "<< rgbcolor.green()
    << ", "<< rgbcolor.blue()
    << ", "<< rgbcolor.alpha() << ")";
}

