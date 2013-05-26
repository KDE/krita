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
     * This is a low level function that expects all the conditions
     * for the @param device be met. Use usual process() methods
     * instead.
     *
     * @param device the paint device to filter
     * @param applyRect the rectangle where the filter is applied
     * @param config the parameters of the filter
     * @param progressUpdater to pass on the progress the filter is making
     */
    virtual void processImpl(KisPaintDeviceSP device,
                             const QRect& applyRect,
                             const KisFilterConfiguration* config,
                             KoUpdater* progressUpdater = 0 ) const = 0;

    /**
     * Filter \p src device and write the result into \p dst device.
     * If \p dst is an alpha color space device, it will get special
     * treatment.
     *
     * @param src the source paint device
     * @param dst the destination paint device
     * @param selection the selection
     * @param applyRect the rectangle where the filter is applied
     * @param config the parameters of the filter
     * @param progressUpdater to pass on the progress the filter is making
     */
    void process(const KisPaintDeviceSP src,
                 KisPaintDeviceSP dst,
                 KisSelectionSP selection,
                 const QRect& applyRect,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater = 0 ) const;


    /**
     * A convenience method for a two-device process() function
     */
    void process(KisPaintDeviceSP device,
                 const QRect& applyRect,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater = 0 ) const;

    bool workWith(const KoColorSpace* cs) const;

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
