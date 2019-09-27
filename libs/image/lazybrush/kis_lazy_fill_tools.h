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

#ifndef __KIS_LAZY_FILL_TOOLS_H
#define __KIS_LAZY_FILL_TOOLS_H

#include "kis_types.h"
#include "kritaimage_export.h"
#include <KoColor.h>

#include <boost/operators.hpp>



class KoColor;

namespace KisLazyFillTools
{
    KRITAIMAGE_EXPORT
    void normalizeAndInvertAlpha8Device(KisPaintDeviceSP dev, const QRect &rect);

    KRITAIMAGE_EXPORT
    void normalizeAlpha8Device(KisPaintDeviceSP dev, const QRect &rect);

    /**
     * Uses Boykov-Kolmogorov Max-Flow/Min-Cut algorithm to split the
     * device \p src into two parts. The first part is defined by \p
     * colorScribble and the second part --- by \p
     * backgroundScribble. In the result of the split the area defined
     * by \p colorScribble in \p resultDevice is filled with \p
     * color. Also the same area in \p maskDevice is filled with a
     * non-null scribble index.
     *
     * \p maskDevice is used for limiting the area used for filling
     *               the color.
     */
    KRITAIMAGE_EXPORT
    void cutOneWay(const KoColor &color,
                   KisPaintDeviceSP src,
                   KisPaintDeviceSP colorScribble,
                   KisPaintDeviceSP backgroundScribble,
                   KisPaintDeviceSP resultDevice,
                   KisPaintDeviceSP maskDevice,
                   const QRect &boundingRect);

    /**
     * Returns one pixel from each connected component of \p src.
     *
     * WARNING: \p src is used as a temporary device, so it will be
     *          cleared(!) after the execution of the algorithm
     */

    KRITAIMAGE_EXPORT
    QVector<QPoint> splitIntoConnectedComponents(KisPaintDeviceSP src, const QRect &boundingRect);

    struct KRITAIMAGE_EXPORT KeyStroke : public boost::equality_comparable<KeyStroke>
    {
        KeyStroke();
        KeyStroke(KisPaintDeviceSP _dev, const KoColor &_color, bool isTransparent = false);

        friend bool operator==(const KeyStroke& t1, const KeyStroke&t2);

        KisPaintDeviceSP dev;
        KoColor color;
        bool isTransparent;
    };

    struct KRITAIMAGE_EXPORT FilteringOptions : public boost::equality_comparable<FilteringOptions>
    {
        FilteringOptions() = default;
        FilteringOptions(bool _useEdgeDetection, qreal _edgeDetectionSize, qreal _fuzzyRadius, qreal _cleanUpAmount);

        friend bool operator==(const FilteringOptions &t1, const FilteringOptions &t2);

        // default values for filtering: disabled
        bool useEdgeDetection = false;
        qreal edgeDetectionSize = 4;
        qreal fuzzyRadius = 0;
        qreal cleanUpAmount = 0.0;
    };
}

#endif /* __KIS_LAZY_FILL_TOOLS_H */
