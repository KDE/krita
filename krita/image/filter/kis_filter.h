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
    static const KoID CategoryAdjust;
    static const KoID CategoryArtistic;
    static const KoID CategoryBlur;
    static const KoID CategoryColors;
    static const KoID CategoryEdgeDetection;
    static const KoID CategoryEmboss;
    static const KoID CategoryEnhance;
    static const KoID CategoryMap;
    static const KoID CategoryNonPhotorealistic;
    static const KoID CategoryOther;

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

protected:

    QString configEntryGroup() const;

};


#endif
