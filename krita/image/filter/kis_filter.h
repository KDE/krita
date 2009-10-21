/*
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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
#ifndef _KIS_FILTER_H_
#define _KIS_FILTER_H_

#include <list>

#include <QString>

#include <klocale.h>

#include "KoID.h"
#include "KoColorSpace.h"

#include "kis_types.h"
#include "kis_base_processor.h"

#include "krita_export.h"

class KisConstProcessingInformation;
class KisProcessingInformation;

/**
 * Basic interface of a Krita filter.
 */
class KRITAIMAGE_EXPORT KisFilter : public KisBaseProcessor
{

public:
    static KoID categoryAdjust();
    static KoID categoryArtistic();
    static KoID categoryBlur();
    static KoID categoryColors();
    static KoID categoryEdgeDetection();
    static KoID categoryEmboss();
    static KoID categoryEnhance();
    static KoID categoryMap();
    static KoID categoryNonPhotorealistic();
    static KoID categoryOther();

public:

    /**
     * Construct a Krita filter
     */
    KisFilter(const KoID& id, const KoID & category, const QString & entry);
    virtual ~KisFilter();

public:

    /**
     * Override this function with the implementation of your filter.
     *
     * XXX: May the filter may assume that src and dst have the same
     * colorspace? (bsar)
     *
     * @param src the source paint device
     * @param dst the destination paint device
     * @param size the size of the area that is filtered
     * @param config the parameters of the filter
     * @param progressUpdater to pass on the progress the filter is making
     */
    virtual void process(KisConstProcessingInformation src,
                         KisProcessingInformation dst,
                         const QSize& size,
                         const KisFilterConfiguration* config,
                         KoUpdater* progressUpdater
                        ) const = 0;

    /**
     * Provided for convenience when no progress reporting is needed.
     */
    virtual void process(KisConstProcessingInformation src,
                         KisProcessingInformation dst,
                         const QSize& size,
                         const KisFilterConfiguration* config
                        ) const;


    /**
     * Provided for convenience only when source and destination are the same
     */
    void process(KisPaintDeviceSP device, const QRect& rect, const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater) const;

    /**
     * Provided for convenience only when source and destination are the same and no progress reporting
     * is necessary.
     */
    void process(KisPaintDeviceSP device, const QRect& rect, const KisFilterConfiguration* config) const;


    bool workWith(const KoColorSpace* cs) const;
    /**
     * Used when threading is used -- the overlap margin is passed to the
     * filter to use to compute pixels, but the margin is not pasted into the
     * resulting image. Use this for convolution filters, for instance.
     *
     * This function is deprecated, use \ref neededRect instead
     */
    KDE_DEPRECATED virtual int overlapMarginNeeded(const KisFilterConfiguration* = 0) const;

    /**
     * Some filters need pixels outside the current processing rect to compute the new
     * value (for instance, convolution filters)
     */
    virtual QRect neededRect(const QRect & rect, const KisFilterConfiguration* = 0) const;

    /**
    * Similar to \ref neededRect: some filters will alter a lot of pixels that are
    * near to each other at the same time. So when you changed a single rectangle
    * in a device, the actual rectangle that will feel the influence of this change
    * might be bigger. Use this function to determine that rect.
     */
    virtual QRect changedRect(const QRect & rect, const KisFilterConfiguration* = 0) const;

protected:

    QString configEntryGroup() const;

};


#endif
