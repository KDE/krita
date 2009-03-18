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
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_types.h"

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

void KisFilter::process(KisConstProcessingInformation src,
                        KisProcessingInformation dst,
                        const QSize& size,
                        const KisFilterConfiguration* config) const
{
    process(src, dst, size, config, 0);
}

void KisFilter::process(KisPaintDeviceSP device, const QRect& rect, const KisFilterConfiguration* config,
                        KoUpdater* progressUpdater) const
{
    KisProcessingInformation info(device, rect.topLeft());
    process(info, info, rect.size(), config, progressUpdater);
}

void KisFilter::process(KisPaintDeviceSP device, const QRect& rect, const KisFilterConfiguration* config) const
{
    KisProcessingInformation info(device, rect.topLeft());
    process(info, info, rect.size(), config, 0);
}


bool KisFilter::workWith(const KoColorSpace* cs) const
{
    Q_UNUSED(cs); return true;
}

int KisFilter::overlapMarginNeeded(const KisFilterConfiguration*) const
{
    return 0;
}

QRect KisFilter::neededRect(const QRect & rect, const KisFilterConfiguration* c) const
{
    QRect rc = rect;
    int margin = overlapMarginNeeded(c);
    rc.setLeft(rect.left() - margin);
    rc.setTop(rect.top() - margin);
    rc.setRight(rect.right() + margin);
    rc.setBottom(rect.bottom() + margin);
    return rc;
}

QRect KisFilter::changedRect(const QRect & rect, const KisFilterConfiguration* c) const
{
    QRect rc = rect;
    int margin = overlapMarginNeeded(c);
    rc.setLeft(rect.left() - margin);
    rc.setTop(rect.top() - margin);
    rc.setRight(rect.right() + margin);
    rc.setBottom(rect.bottom() + margin);
    return rc;
}

