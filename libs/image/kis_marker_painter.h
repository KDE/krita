/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_MARKER_PAINTER_H
#define __KIS_MARKER_PAINTER_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kritaimage_export.h"

class KoColor;


class KRITAIMAGE_EXPORT KisMarkerPainter
{
public:
    /// Any number bigger than this or lower than -this is considered invalid
    static const qint32 ValidNumberRangeValue = 2140000000; // bit less than max value of int


    KisMarkerPainter(KisPaintDeviceSP device, const KoColor &color);
    ~KisMarkerPainter();

    void fillFullCircle(const QPointF &center, qreal radius);
    void fillHalfBrushDiff(const QPointF &p1, const QPointF &p2, const QPointF &p3,
                           const QPointF &center, qreal radius);

    void fillCirclesDiff(const QPointF &c1, qreal r1,
                         const QPointF &c2, qreal r2);

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    /// This method is to check whether the number is not infinite
    /// or negative infinite with some epsilon
    /// (@see ValidNumberRangeValue)
    /// @param number value entered by the user
    /// @return true if number is in range, false otherwise
    bool isNumberInValidRange(qint32 number);


    /// This method is to check whether the rectangle has only valid numbers
    /// as values for x, y, height and width.
    /// If values are not valid, Sequential Iterator can give incorrect values.
    /// (@see isNumberInValidRange, ValidNumberRangeValue)
    /// @param number value entered by the user
    /// @return true if rect's values is in range, false otherwise
    bool isRectInValidRange(const QRect &rect);
};

#endif /* __KIS_MARKER_PAINTER_H */
