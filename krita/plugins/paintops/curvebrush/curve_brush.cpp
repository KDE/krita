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
const int _POINTS = 4;

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

CurveBrush::CurveBrush() : m_head(0), m_counter(1), m_incr(1)
{
    for (int i = 0; i < _POINTS; i++) {
        m_points[i] = QPointF(0, 0);
    }

}

CurveBrush::~CurveBrush()
{
}


void CurveBrush::rotatePoints(int *x, int *y, qreal centerX, qreal centerY, qreal angle)
{
    qreal mx = *x - centerX;
    qreal my = *y - centerY;

    qreal rotX = sin(angle) * mx + cos(angle) * my;
    qreal rotY = cos(angle) * mx + sin(angle) * my;

    rotX += centerX;
    rotY += centerY;
    *x = int (rotX + 0.5);
    *y = int (rotY + 0.5);

}

inline void CurveBrush::normalizePoint(QPointF &p)
{
    p.setX(p.x() / m_image->width());
    p.setY(p.y() / m_image->height());
}

QPointF CurveBrush::getLinearBezier(const QPointF &p1, const QPointF &p2, qreal u)
{
    qreal rx = (1.0 - u) * p1.x() + u * p2.x();
    qreal ry = (1.0 - u) * p1.y() + u * p2.y();
    return QPointF(rx, ry);
}

QPointF CurveBrush::getQuadraticBezier(const QPointF &p0, const QPointF &p1, const QPointF &p2, qreal u)
{
    qreal rx =
        pow((1.0 - u), 2)   * p0.x() +
        2 * u * (1.0 - u)    * p1.x() +
        pow(u, 2)           * p2.x();

    qreal ry =
        pow((1.0 - u), 2)   * p0.y() +
        2 * u * (1.0 - u)    * p1.y() +
        pow(u, 2)           * p2.y();


    return QPointF(rx, ry);
}


QPointF CurveBrush::getCubicBezier(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3, qreal u)
{
    qreal rx =
        pow((1.0 - u), 3) * p0.x() +
        3.0  * u * pow((1.0 - u), 2) * p1.x() +
        3.0  * pow(u, 2) * (1.0 - u) * p2.x() +
        pow(u, 3) * p3.x();

    qreal ry =
        pow((1.0 - u), 3) * p0.y() +
        3.0  * u * pow((1.0 - u), 2) * p1.y() +
        3.0  * pow(u, 2) * (1.0 - u) * p2.y() +
        pow(u, 3) * p3.y();


    return QPointF(rx, ry);
}

/*
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
            result = getBezier ( p0,p1,p2,p3, ( qreal ) i/ ( qreal ) steps );
            paintX =  result.x() * m_image->width();
            paintY =  result.y() * m_image->height();


                moveX = paintX;
                moveY = paintY;
                rotatePoints ( &paintX, &paintY,pi1.pos().x(),pi1.pos().y(), pi1.angle() +1.57 );

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

    m_counter++;
}
*/


void CurveBrush::paintLine(KisPaintDeviceSP dab, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    // Initialization
    if (!m_image) return;

    int w = m_image->width();
    int h = m_image->height();

    qreal maxDist = sqrt(w * w + h * h);

    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;
    qreal angle = atan2(dy, dx);
    qreal distance = sqrt(dx * dx + dy * dy);

    KisRandomAccessor accessor = dab->createRandomAccessor((int) x1, (int) y1);
    m_writeAccessor = &accessor;

    KisRandomAccessor accessor2 = layer->createRandomAccessor((int) x1, (int) y1);
    m_readAccessor = &accessor2;

    m_layer = layer;
    m_dab = dab;
    m_pixelSize = dab->colorSpace()->pixelSize();

    srand(time(0));
    int steps = 200;
    KoColor pcolor = m_inkColor;
    // end of initialization

    qreal randX, randY;
    randX = ((x1 + x2) / 2.0);
    randY = ((y1 + y2) / 2.0);
    qreal dNorm = distance / maxDist;

    qreal clen;

    /*
        dbgPlugins << "mode:" << m_mode;
        dbgPlugins << "min:" << m_minimalDistance;
        dbgPlugins << "interval:" << m_interval;
    */

    switch (m_mode) {
    case 1: clen = ((drand48() * m_interval) - m_interval);
        break;
    case 2: clen = drand48() * m_counter;
        break;
    case 3: clen = m_counter;
        break;
    }

    if (distance > m_minimalDistance) {

        if (x1 == x2) {
            randX += clen;
        } else if (y1 == y2) {
            randY += clen;
        } else if (fabs(dx) > fabs(dy)) {
            // horizontal
            randY += clen;

        } else {
            randX += clen;
        }

    }

    QPointF p0 = pi1.pos();
    QPointF p2 = pi2.pos();
    QPointF p1(randX, randY);

    dbgPlugins << "x1:y1 -- x2:y2" << p0 << p2;
    dbgPlugins << p1;

    QPointF result;
    normalizePoint(p0);
    normalizePoint(p1);
    normalizePoint(p2);

    qreal paintX, paintY;
    int moveX, moveY;

    for (int i = 0 ; i <= steps; i++) {
        result = getQuadraticBezier(p0, p1, p2, (qreal) i / (qreal) steps);
        paintX =  result.x() * m_image->width();
        paintY =  result.y() * m_image->height();


        int ipx = int (paintX);
        int ipy = int (paintY);
        qreal fx = paintX - ipx;
        qreal fy = paintY - ipy;

        qreal MAX_OPACITY = 255;

        int btl = qRound((1 - fx) * (1 - fy) * MAX_OPACITY);
        int btr = qRound((fx)   * (1 - fy) * MAX_OPACITY);
        int bbl = qRound((1 - fx) * (fy)   * MAX_OPACITY);
        int bbr = qRound((fx)   * (fy)   * MAX_OPACITY);

        pcolor.setOpacity(btl);
        m_writeAccessor->moveTo(ipx  , ipy);
        if (m_layer->colorSpace()->alpha(m_writeAccessor->rawData()) < pcolor.opacity()) {
            memcpy(m_writeAccessor->rawData(), pcolor.data(), m_pixelSize);
        }

        pcolor.setOpacity(btr);
        m_writeAccessor->moveTo(ipx + 1, ipy);
        if (m_layer->colorSpace()->alpha(m_writeAccessor->rawData()) < pcolor.opacity()) {
            memcpy(m_writeAccessor->rawData(), pcolor.data(), m_pixelSize);
        }

        pcolor.setOpacity(bbl);
        m_writeAccessor->moveTo(ipx, ipy + 1);
        if (m_layer->colorSpace()->alpha(m_writeAccessor->rawData()) < pcolor.opacity()) {
            memcpy(m_writeAccessor->rawData(), pcolor.data(), m_pixelSize);
        }

        pcolor.setOpacity(bbr);
        m_writeAccessor->moveTo(ipx + 1, ipy + 1);
        if (m_layer->colorSpace()->alpha(m_writeAccessor->rawData()) < pcolor.opacity()) {
            memcpy(m_writeAccessor->rawData(), pcolor.data(), m_pixelSize);
        }

    }


    m_counter += m_incr;
    if (abs(m_counter) - 1 == m_interval) {
        m_incr *= -1;
    }

}





void CurveBrush::addPoint(QPointF p)
{
    if ((m_head) == _POINTS) {
        removeLast();
    }
    m_points[m_head] = p;
    m_head++;
}

void CurveBrush::removeLast()
{
    for (int i = 1; i < _POINTS; i++) {
        m_points[i-1] = m_points[i];
    }
    m_head--;
}


void CurveBrush::debugColor(const quint8* data)
{
    QColor rgbcolor;
    m_dab->colorSpace()->toQColor(data, &rgbcolor);
    dbgPlugins << "RGBA: ("
    << rgbcolor.red()
    << ", " << rgbcolor.green()
    << ", " << rgbcolor.blue()
    << ", " << rgbcolor.alpha() << ")";
}

