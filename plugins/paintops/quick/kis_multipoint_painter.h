/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_MULTIPOINT_PAINTER_H
#define __KIS_MULTIPOINT_PAINTER_H

#include "kritaquickpaintop_export.h"

#include <QScopedPointer>
#include <QPointF>
#include <QVector>
#include <QRect>

#include "kis_types.h"

class KoColor;


class KRITAQUICKPAINTOP_EXPORT KisMultipointPainter
{
public:
    KisMultipointPainter();
    ~KisMultipointPainter();

    struct Point {
        Point()
        : rx(0.0), ry(0.0), opacity(0.0) {}

    Point(QPointF _pos, float _rx, float _ry, float _opacity)
        : pos(_pos), rx(_rx), ry(_ry), opacity(_opacity) {}

        QPointF pos;
        float rx;
        float ry;
        float opacity;
    };

    void setPoints(const QVector<Point> &points);

    QRect boundingRect() const;

    void paintPoints(KisPaintDeviceSP dev, const KoColor &color);
    void paintPoints2(KisPaintDeviceSP dev, const KoColor &color);
    void paintPoints3(KisPaintDeviceSP dev, const KoColor &color);


private:
    friend class KisQuickopTest;

    struct PointRow {
        int start;
        int end;
        const QVector<float> *dxValues;
        float dyValue;

        float baseUnitValue;
    };
public:
    struct PointArea {
        int xStart;
        int xEnd;
        int yStart;
        int yEnd;

        float baseUnitValue;

        QVector<float> dxValues;
        QVector<float> dyValues;

        void initDxDy(const QPointF &center, float rx, float ry);
        PointRow fillRow(int i) const;
    };

    struct MergedArea {
        int xStart;
        int xEnd;
        int yStart;
        int yEnd;

        //QVector<float> baseUnitValues;

        float baseUnitValue;
        QVector<float> dxValues;
        QVector<float> dyValues;

        QVector<const PointArea*> activePoints;
    };

    struct CompositeRow {
        int start;
        int end;
        QVector<PointRow> rows;
    };
public:
    void calcMergedAreas();
    QVector<MergedArea> fetchMergedAreas() const;

private:
    CompositeRow getCompositeRow(int y);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MULTIPOINT_PAINTER_H */
