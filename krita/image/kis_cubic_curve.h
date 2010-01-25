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

#ifndef _KIS_CUBIC_CURVE_H_
#define _KIS_CUBIC_CURVE_H_

#include<QList>
#include<QVector>
#include<QVariant>

#include <krita_export.h>

class QPointF;

/**
 * Hold the data for a cubic curve.
 */
class KRITAIMAGE_EXPORT KisCubicCurve
{
public:
    KisCubicCurve();
    KisCubicCurve(const QList<QPointF>& points);
    KisCubicCurve(const QVector<QPointF>& points);
    KisCubicCurve(const KisCubicCurve& curve);
    KisCubicCurve& operator=(const KisCubicCurve& curve);
    bool operator==(const KisCubicCurve& curve) const;
public:
    qreal value(qreal x) const;
    QList<QPointF> points() const;
    void setPoints(const QList<QPointF>& points);
    void setPoint(int idx, const QPointF& point);
    /**
     * Add a point to the curve, the list of point is always sorted.
     * @return the index of the inserted point
     */
    int addPoint(const QPointF& point);
    void removePoint(int idx);
public:
    QVector<quint16> uint16Transfer(int size = 256) const;
    QVector<qreal> floatTransfer(int size = 256) const;
public:
    QString toString() const;
    void fromString(const QString&);
private:
    struct Data;
    struct Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KisCubicCurve);

#endif
