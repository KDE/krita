/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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


#ifndef KIS_CURVE_H
#define KIS_CURVE_H

#include <QPointF>
#include <QList>

#include "kis_shared.h"

//#include <krita_export.h>

//const QString DEFAULT_CURVE_STRING = "0,0;1,1;";

/**
 * Interface for classes holding curves
 */
class /*KRITAIMAGE_EXPORT*/ KisCurve : public KisShared
{
public:
    KisCurve();
    virtual ~KisCurve();
//    bool operator==(const KisCurve& curve) const;

    virtual qreal value(qreal x) const = 0;
    virtual QList<QPointF> points() const = 0;
    virtual void setPoints(const QList<QPointF>& points) = 0;
    virtual void setPoint(int idx, const QPointF& point) = 0;

    /**
     * Add a point to the curve, the list of point is always sorted.
     * @return the index of the inserted point
     */
    virtual int addPoint(const QPointF& point) = 0;
    virtual void removePoint(int idx) = 0;

    virtual QVector<quint16> uint16Transfer(int size = 256) const = 0;
    virtual QVector<qreal> floatTransfer(int size = 256) const = 0;

    /// must return a string in the format "ClassName:data", where data is a substring, that can be passed to the constructor.
    virtual QString toString() const = 0;

    static KisCurve* fromString(const QString&);
    static bool pointCompare (const QPointF &p1, const QPointF &p2)
    {
        if(p1.x()<p2.x()) return true;
        if(p1.x()==p2.x() && p1.y()<p2.y()) return true;
        return false;
    }

protected:
    static const int UINT16_TOP_BOUND = 0xFFFF;
    static const int UINT16_BOTTOM_BOUND = 0x0;

    static const qreal REAL_TOP_BOUND = 1.0;
    static const qreal REAL_BOTTOM_BOUND = 0.0;
};

#endif // KIS_CURVE_H
