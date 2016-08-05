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

class KoColor;


namespace KisLazyFillTools
{
    /**
     * Uses Boykov-Kolmogorov Max-Flow/Min-Cut algorithm to split the
     * device \src into two parts. The first part is defined by \p
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
                   KisPaintDeviceSP maskDevice);

    /**
     * Returns one pixel from each connected component of \p src.
     *
     * WARNING: \p src is used as a temporary device, so it will be
     *          cleared(!) after the execution of the algorithm
     */

    KRITAIMAGE_EXPORT
    QVector<QPoint> splitIntoConnectedComponents(KisPaintDeviceSP src);
};

#endif /* __KIS_LAZY_FILL_TOOLS_H */
