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

const KoID KisFilter::CategoryAdjust = KoID("adjust_filters", i18n("Adjust"));
const KoID KisFilter::CategoryArtistic = KoID("artistic_filters", i18n("Artistic"));
const KoID KisFilter::CategoryBlur = KoID("blur_filters", i18n("Blur"));
const KoID KisFilter::CategoryColors = KoID("color_filters", i18n("Colors"));
const KoID KisFilter::CategoryEdgeDetection = KoID("edge_filters", i18n("Edge Detection"));
const KoID KisFilter::CategoryEmboss = KoID("emboss_filters", i18n("Emboss"));
const KoID KisFilter::CategoryEnhance = KoID("enhance_filters", i18n("Enhance"));
const KoID KisFilter::CategoryMap = KoID("map_filters", i18n("Map"));
const KoID KisFilter::CategoryNonPhotorealistic = KoID("nonphotorealistic_filters", i18n("Non-photorealistic"));
const KoID KisFilter::CategoryOther = KoID("other_filters", i18n("Other"));


KisFilter::KisFilter(const KoID& id, const KoID & category, const QString & entry)
        : KisBaseProcessor(id, category, entry)
{
    setBookmarkManager(new KisBookmarkedConfigurationManager(configEntryGroup(),
                       new KisFilterConfigurationFactory(id.id(), 1)));
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

QString KisFilter::configEntryGroup() const
{
    return id() + "_filter_bookmarks";
}

bool KisFilter::workWith(const KoColorSpace* cs) const
{
    Q_UNUSED(cs); return true;
}
