/* Copyright (c) Dmitry Kazakov <dimula73@gmail.com>, 2009
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_ASYNC_MERGER_H
#define __KIS_ASYNC_MERGER_H

#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_paint_device.h"

class QRect;
class KisBaseRectsWalker;

class KRITAIMAGE_EXPORT KisAsyncMerger
{
public:
    void startMerge(KisBaseRectsWalker &walker, bool notifyClones = true);

private:
    inline void resetProjection();
    inline void setupProjection(KisProjectionLeafSP currentLeaf, const QRect& rect, bool useTempProjection);
    inline void writeProjection(KisProjectionLeafSP topmostLeaf, bool useTempProjection, QRect rect);
    inline bool compositeWithProjection(KisProjectionLeafSP leaf, const QRect &rect);
    inline void doNotifyClones(KisBaseRectsWalker &walker);

private:
    /**
     * The place where intermediate results of layer's merge
     * are going. It may be equal to m_finalProjection. In
     * this case the projection will be written directly to
     * the original of the parent layer
     */
    KisPaintDeviceSP m_currentProjection;

    /**
     * The final destination of every projection of all
     * the layers. It is equal to the original of a parental
     * node.
     * NOTE: This pointer is cached here to avoid
     *       race conditions
     */
    KisPaintDeviceSP m_finalProjection;

    /**
     * Creation of the paint device is quite expensive, so we'll just
     * save the pointer to our temporary device here and will get it when
     * needed. This variable must not be used anywhere out of
     * setupProjection()
     */
    KisPaintDeviceSP m_cachedPaintDevice;
};


#endif /* __KIS_ASYNC_MERGER_H */

