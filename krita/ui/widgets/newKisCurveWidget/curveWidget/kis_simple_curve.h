/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#ifndef KIS_SIMPLE_CURVE_H
#define KIS_SIMPLE_CURVE_H

#include "kis_curve.h"

#include <QPainterPath>

class KisSimpleCurve : public KisCurve
{
public:
    KisSimpleCurve();

    virtual QList<QPointF> points() const;
    virtual void setPoints(const QList<QPointF>& points);
    virtual void setPoint(int idx, const QPointF& point);

    /**
     * Add a point to the curve, the list of point is always sorted.
     * @return the index of the inserted point
     */
    virtual int addPoint(const QPointF& point);
    virtual void removePoint(int idx);

    /// implementation should return its classname. this is used for toString().
    virtual QString className() const = 0;

    /// returns a string in the format "className():x0,y0;x1,y1;x2,y2;.."
    virtual QString toString() const;

    QVector<quint16> uint16Transfer(int size = 256) const;
    QVector<qreal> floatTransfer(int size = 256) const;
    qreal value(qreal x) const;
    QPainterPath painterPath() const;

protected:
    template <typename T>
    QVector<T> transfer(int size) const;
    virtual void updatePainterPath() = 0;

    QPainterPath m_painterPath;
    /// the points are in the range of 0.0 to 1.0
    QList<QPointF> m_points;
};

#endif // KIS_SIMPLE_CURVE_H
