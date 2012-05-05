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

#include <kis_random_accessor_ng.h>

#include <cmath>
#include <ctime>

#include "kis_curve_line_option.h"

const int STEPS = 200;

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

CurveBrush::CurveBrush() :
    m_painter(0),
    m_branch(0)
{
    srand48(time(0));
#if QT_VERSION >= 0x040700
    m_pens.reserve(1024);
#endif
}

CurveBrush::~CurveBrush()
{
    delete m_painter;
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
        if (cs->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }

        color.setOpacity(btr);
        m_writeAccessor->moveTo(ipx + 1, ipy);
        if (cs->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }

        color.setOpacity(bbl);
        m_writeAccessor->moveTo(ipx, ipy + 1);
        if (cs->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }

        color.setOpacity(bbr);
        m_writeAccessor->moveTo(ipx + 1, ipy + 1);
        if (cs->opacityU8(m_writeAccessor->rawData()) < color.opacityU8()) {
            memcpy(m_writeAccessor->rawData(), color.data(), m_pixelSize);
        }
}

void CurveBrush::strokePens(QPointF pi1, QPointF pi2, KisPainter &/*painter*/) {
    if (m_pens.isEmpty()) {
        m_pens.append(Pen(pi1,0.0,1.0));
    }

    qreal dx = pi2.x() - pi1.x();
    qreal dy = pi2.y() - pi1.y();
    for (int i = 0; i < m_pens.length(); i++){
        Pen &pen = m_pens[i];

        QPointF endPoint(dx, dy);

        QPainterPath path;
        path.moveTo(0,0);
        path.lineTo(dx, dy);

        QTransform transform;
        transform.reset();
        transform.translate(pen.pos.x(), pen.pos.y());
        transform.scale(pen.scale, pen.scale);
        transform.rotate(pen.rotation);

        path = transform.map(path);
        //m_painter->drawPainterPath(path, QPen(Qt::white, 1.0));

        endPoint = transform.map(endPoint);
        m_painter->drawThickLine(pen.pos, endPoint, 1.0, 1.0);
        pen.pos = endPoint;
    }

    qreal branchThreshold = 0.5;
    if ((m_branch * drand48() > branchThreshold) && (m_pens.length() < 1024)){
         int index = floor(drand48() * (m_pens.length()-1));

         m_newPen.pos = m_pens.at(index).pos;
         m_newPen.rotation = drand48() * M_PI/32;//atan(dy/dx) + (drand48() - 0.5) * M_PI/32;
         m_newPen.scale = drand48() * m_pens.at(index).scale;
         m_pens.append(m_newPen);
         qDebug() << m_pens.length();
         m_branch = 0;
      } else {
         m_branch++;
      }
}

