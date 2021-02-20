/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
