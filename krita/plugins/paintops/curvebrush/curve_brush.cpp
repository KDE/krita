/*
 *  Copyright (c) 2008,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <kis_random_accessor.h>

#include <cmath>
#include <ctime>

const int STEPS = 200; 

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

CurveBrush::CurveBrush() : m_counter(1), m_increment(1)
{
    srand48(time(0));
}

CurveBrush::~CurveBrush()
{
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


void CurveBrush::paintLine(KisPaintDeviceSP dab, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;
    //qreal angle = atan2(dy, dx);
    qreal distance = sqrt(dx * dx + dy * dy);

    KisRandomAccessor accessor = dab->createRandomAccessor((int) x1, (int) y1);
    m_writeAccessor = &accessor;

    m_layer = layer;
    m_pixelSize = dab->colorSpace()->pixelSize();
    
    KoColor pcolor = m_painter->paintColor();
    // end of initialization

    qreal midPointX, midPointY;
    midPointX = ((x1 + x2) / 2.0);
    midPointY = ((y1 + y2) / 2.0);
    
    qreal midPointOffset = 0.0;
    switch (m_mode) {
    case 1: midPointOffset = ((drand48() * m_interval) - m_interval);
        break;
    case 2: midPointOffset = drand48() * m_counter;
        break;
    case 3: midPointOffset = m_counter;
        break;
    }

    if (distance > m_minimalDistance) {
        if (x1 == x2) {
            midPointX += midPointOffset;
        } else if (y1 == y2) {
            midPointY += midPointOffset;
        } else if (fabs(dx) > fabs(dy)) {
            // horizontal
            midPointY += midPointOffset;
        } else {
            midPointX += midPointOffset;
        }
    }

    QPointF p0 = pi1.pos();
    QPointF p2 = pi2.pos();
    QPointF p1(midPointX, midPointY);

    QPointF result;
    for (int i = 0 ; i <= STEPS; i++) {
        result = getQuadraticBezier(p0, p1, p2, i / (qreal)STEPS);
        putPixel(result,pcolor);
    }

    m_counter += m_increment;
    if (abs(m_counter) - 1 == m_interval) {
        m_increment *= -1;
    }

}

void CurveBrush::putPixel(QPointF pos, KoColor &color)
{
        int ipx = int (pos.x());
        int ipy = int (pos.y());
        qreal fx = pos.x() - ipx;
        qreal fy = pos.y() - ipy;

        qreal btl = (1 - fx) * (1 - fy);
        qreal btr = (fx)   * (1 - fy);
        qreal bbl = (1 - fx) * (fy);
        qreal bbr = (fx)   * (fy);

        color.setOpacity(btl);
        m_writeAccessor->moveTo(ipx  , ipy);
        if (m_layer->colorSpace()->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }

        color.setOpacity(btr);
        m_writeAccessor->moveTo(ipx + 1, ipy);
        if (m_layer->colorSpace()->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }

        color.setOpacity(bbl);
        m_writeAccessor->moveTo(ipx, ipy + 1);
        if (m_layer->colorSpace()->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }

        color.setOpacity(bbr);
        m_writeAccessor->moveTo(ipx + 1, ipy + 1);
        if (m_layer->colorSpace()->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }
}

