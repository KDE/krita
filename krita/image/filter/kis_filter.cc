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

#include "filter/kis_filter.h"

#include <QString>

#include "kis_bookmarked_configuration_manager.h"
#include "filter/kis_filter_configuration.h"
#include "kis_processing_information.h"
#include "kis_transaction.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_types.h"
#include <kis_painter.h>

KoID KisFilter::categoryAdjust()
{
    return KoID("adjust_filters", i18n("Adjust"));
}

KoID KisFilter::categoryArtistic()
{
    return KoID("artistic_filters", i18n("Artistic"));
}

KoID KisFilter::categoryBlur()
{
    return KoID("blur_filters", i18n("Blur"));
}

KoID KisFilter::categoryColors()
{
    return KoID("color_filters", i18n("Colors"));
}

KoID KisFilter::categoryEdgeDetection()
{
    return KoID("edge_filters", i18n("Edge Detection"));
}

KoID KisFilter::categoryEmboss()
{
    return KoID("emboss_filters", i18n("Emboss"));
}

KoID KisFilter::categoryEnhance()
{
    return KoID("enhance_filters", i18n("Enhance"));
}

KoID KisFilter::categoryMap()
{
    return KoID("map_filters", i18n("Map"));
}

KoID KisFilter::categoryNonPhotorealistic()
{
    return KoID("nonphotorealistic_filters", i18n("Non-photorealistic"));
}

KoID KisFilter::categoryOther()
{
    return KoID("other_filters", i18n("Other"));
}


KisFilter::KisFilter(const KoID& _id, const KoID & category, const QString & entry)
        : KisBaseProcessor(_id, category, entry)
{
    init(id() + "_filter_bookmarks");
}

KisFilter::~KisFilter()
{
}

void KisFilter::process(const KisPaintDeviceSP src,
                KisPaintDeviceSP dst,
                KisSelectionSP sel,
                const QRect& applyRect,
                const KisFilterConfiguration* config,
                KoUpdater* progressUpdater ) const
{
    // we're not filtering anything, so return.
    if (applyRect.isEmpty()) {
        return;
    }
    QRect nR = neededRect(applyRect, config);
    // Create a temporary device that will be filtered, sharing the source data
    KisPaintDeviceSP temporary;
    KisTransaction *transaction = 0;

    if(src == dst && sel == 0)
    {
        temporary = src;
    }
    else {
        temporary = new KisPaintDevice(src->colorSpace());
        temporary->makeCloneFromRough(src, nR);
        transaction = new KisTransaction("", temporary);
    }
    // Filter
    process(temporary, applyRect, config, progressUpdater);

    // Copy on destination, respecting the selection
    if(temporary != src)
    {
        delete transaction;
        KisPainter p(dst);
        p.setCompositeOp(COMPOSITE_COPY);
        p.setSelection(sel);
        p.bitBlt(applyRect.topLeft(), temporary, applyRect);
    }
}

bool KisFilter::workWith(const KoColorSpace* cs) const
{
    Q_UNUSED(cs); return true;
}

QRect KisFilter::neededRect(const QRect & rect, const KisFilterConfiguration* c) const
{
    Q_UNUSED(c);
    return rect;
}

QRect KisFilter::changedRect(const QRect & rect, const KisFilterConfiguration* c) const
{
    Q_UNUSED(c);
    return rect;
}

