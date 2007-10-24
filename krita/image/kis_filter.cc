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
#include "kis_filter.h"

#include <QString>

#include "kis_bookmarked_configuration_manager.h"
#include "kis_filter_configuration.h"
#include "kis_filter_processing_information.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_types.h"

const KoID KisFilter::CategoryAdjust = KoID("adjust_filters", i18n("Adjust"));
const KoID KisFilter::CategoryArtistic = KoID("artistic_filters", i18n("Artistic"));
const KoID KisFilter::CategoryBlur = KoID("blur_filters", i18n("Blur"));
const KoID KisFilter::CategoryColors = KoID("colors_filters", i18n("Colors"));
const KoID KisFilter::CategoryEdgeDetection = KoID("edge_filters", i18n("Edge Detection"));
const KoID KisFilter::CategoryEmboss = KoID("emboss_filters", i18n("Emboss"));
const KoID KisFilter::CategoryEnhance = KoID("enhance_filters", i18n("Enhance"));
const KoID KisFilter::CategoryMap = KoID("map_filters", i18n("Map"));
const KoID KisFilter::CategoryNonPhotorealistic = KoID("nonphotorealistic_filters", i18n("Non-photorealistic"));
const KoID KisFilter::CategoryOther = KoID("other_filters", i18n("Other"));

struct KisFilter::Private {
    Private()
        : bookmarkManager(0)
    {
    }

    KisBookmarkedConfigurationManager* bookmarkManager;

    KoID id;
    KoID category; // The category in the filter menu this filter fits
    QString entry; // the i18n'ed accelerated menu text
};

KisFilter::KisFilter(const KoID& id, const KoID & category, const QString & entry)
    : KisProgressSubject(0, id.id().toLatin1())
    , d(new Private)
{
    setBookmarkManager(new KisBookmarkedConfigurationManager(configEntryGroup(), new KisFilterConfigurationFactory(id.id(), 1) ));
    d->id = id;
    d->category = category;
    d->entry = entry;
}

KisFilterConfiguration * KisFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    return new KisFilterConfiguration(id(), 0);
}

KisFilterConfiguration * KisFilter::defaultConfiguration(const KisPaintDeviceSP pd) const
{
    KisFilterConfiguration* fc = 0;
    if(bookmarkManager())
    {
        fc = dynamic_cast<KisFilterConfiguration*>(bookmarkManager()->defaultConfiguration());
    }
    if(not fc or not fc->isCompatible(pd) )
    {
        fc = factoryConfiguration(pd);
    }
    return fc;
}

KisFilterConfigWidget * KisFilter::createConfigurationWidget(QWidget *, const KisPaintDeviceSP)
{
    return 0;
}

QRect KisFilter::enlargeRect(QRect rect, const KisFilterConfiguration* c) const {
    int margin = overlapMarginNeeded(c);
    rect.setLeft(rect.left() - margin);
    rect.setTop(rect.top() - margin);
    rect.setRight(rect.right() + margin);
    rect.setBottom(rect.bottom() + margin);
    return rect;
}

void KisFilter::process(KisPaintDeviceSP device, const QRect& rect, const KisFilterConfiguration* config,
                 KoProgressUpdater* progressUpdater)
{
    KisFilterProcessingInformation info(device, rect.topLeft());
    process(info, info, rect.size(), config, progressUpdater);
}

KisBookmarkedConfigurationManager* KisFilter::bookmarkManager()
{
    return d->bookmarkManager;
}

const KisBookmarkedConfigurationManager* KisFilter::bookmarkManager() const
{
    return d->bookmarkManager;
}

void KisFilter::setBookmarkManager(KisBookmarkedConfigurationManager* bm)
{
    delete d->bookmarkManager;
    d->bookmarkManager = bm;
}

QString KisFilter::id() const { return d->id.id(); }
QString KisFilter::name() const { return d->id.name(); }

KoID KisFilter::menuCategory() const { return d->category; }

QString KisFilter::menuEntry() const { return d->entry; }

#include "kis_filter.moc"
