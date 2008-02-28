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

#include "kis_base_processor.h"

#include <QString>

#include "kis_bookmarked_configuration_manager.h"
#include "filter/kis_filter_configuration.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_types.h"

struct KisBaseProcessor::Private {
    Private()
        : bookmarkManager(0), supportsPainting(false), supportsPreview(true), supportsAdjustmentLayers(false), supportsIncrementalPainting(true), supportsThreading(true), colorSpaceIndependence(FULLY_INDEPENDENT)
    {
    }

    KisBookmarkedConfigurationManager* bookmarkManager;

    KoID id;
    KoID category; // The category in the filter menu this filter fits
    QString entry; // the i18n'ed accelerated menu text
    bool supportsPainting;
    bool supportsPreview;
    bool supportsAdjustmentLayers;
    bool supportsIncrementalPainting;
    bool supportsThreading;
    ColorSpaceIndependence colorSpaceIndependence;
};

KisBaseProcessor::KisBaseProcessor(const KoID& id, const KoID & category, const QString & entry)
    : d(new Private)
{
    d->id = id;
    d->category = category;
    d->entry = entry;
}

KisBaseProcessor::~KisBaseProcessor()
{
    delete d->bookmarkManager;
    delete d;
}

KisFilterConfiguration * KisBaseProcessor::factoryConfiguration(const KisPaintDeviceSP) const
{
    return new KisFilterConfiguration(id(), 0);
}

KisFilterConfiguration * KisBaseProcessor::defaultConfiguration(const KisPaintDeviceSP pd) const
{
    KisFilterConfiguration* fc = 0;
    if(bookmarkManager())
    {
        fc = dynamic_cast<KisFilterConfiguration*>(bookmarkManager()->defaultConfiguration());
    }
    if(!fc || !fc->isCompatible(pd) )
    {
        fc = factoryConfiguration(pd);
    }
    return fc;
}

KisFilterConfigWidget * KisBaseProcessor::createConfigurationWidget(QWidget *, const KisPaintDeviceSP) const
{
    return 0;
}

QRect KisBaseProcessor::enlargeRect(const QRect & rect, const KisFilterConfiguration* c) const {
    QRect rc = rect;
    int margin = overlapMarginNeeded(c);
    rc.setLeft(rect.left() - margin);
    rc.setTop(rect.top() - margin);
    rc.setRight(rect.right() + margin);
    rc.setBottom(rect.bottom() + margin);
    return rc;
}


KisBookmarkedConfigurationManager* KisBaseProcessor::bookmarkManager()
{
    return d->bookmarkManager;
}

const KisBookmarkedConfigurationManager* KisBaseProcessor::bookmarkManager() const
{
    return d->bookmarkManager;
}

void KisBaseProcessor::setBookmarkManager(KisBookmarkedConfigurationManager* bm)
{
    delete d->bookmarkManager;
    d->bookmarkManager = bm;
}

int KisBaseProcessor::overlapMarginNeeded( const KisFilterConfiguration* ) const { return 0; }

QString KisBaseProcessor::id() const { return d->id.id(); }

QString KisBaseProcessor::name() const { return d->id.name(); }

KoID KisBaseProcessor::menuCategory() const { return d->category; }

QString KisBaseProcessor::menuEntry() const { return d->entry; }

bool KisBaseProcessor::supportsPainting() const
{
    return d->supportsPainting;
}

bool KisBaseProcessor::supportsPreview() const
{
    return d->supportsPreview;
}

bool KisBaseProcessor::supportsAdjustmentLayers() const
{
    return d->supportsAdjustmentLayers;
}

bool KisBaseProcessor::supportsIncrementalPainting() const
{
    return d->supportsIncrementalPainting;
}

bool KisBaseProcessor::supportsThreading() const
{
    return d->supportsThreading;
}

ColorSpaceIndependence KisBaseProcessor::colorSpaceIndependence() const
{
    return d->colorSpaceIndependence;
}

void KisBaseProcessor::setSupportsPainting(bool v)
{
    d->supportsPainting = v;
}
void KisBaseProcessor::setSupportsPreview(bool v)
{
    d->supportsPreview = v;
}
void KisBaseProcessor::setSupportsAdjustmentLayers(bool v)
{
    d->supportsAdjustmentLayers = v;
}
void KisBaseProcessor::setSupportsIncrementalPainting(bool v)
{
    d->supportsIncrementalPainting = v;
}
void KisBaseProcessor::setSupportsThreading(bool v)
{
    d->supportsThreading = v;
}

void KisBaseProcessor::setColorSpaceIndependence(ColorSpaceIndependence v)
{
    d->colorSpaceIndependence = v;
}
