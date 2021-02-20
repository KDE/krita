/* SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KIS_ASYNC_MERGER_H
#define __KIS_ASYNC_MERGER_H

#include "kritaimage_export.h"
#include "kis_types.h"

class QRect;
class KisBaseRectsWalker;

class KRITAIMAGE_EXPORT KisAsyncMerger
{
public:
    void startMerge(KisBaseRectsWalker &walker, bool notifyClones = true);

private:
    inline void resetProjection();
    inline void setupProjection(KisProjectionLeafSP currentLeaf, const QRect& rect, bool useTempProjection);
    inline void writeProjection(KisProjectionLeafSP topmostLeaf, bool useTempProjection, const QRect &rect);
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

