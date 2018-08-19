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

#include <KoCompositeOpRegistry.h>
#include "kis_bookmarked_configuration_manager.h"
#include "filter/kis_filter_configuration.h"
#include "kis_processing_information.h"
#include "kis_transaction.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_types.h"
#include <kis_painter.h>
#include <KoUpdater.h>

KisFilter::KisFilter(const KoID& _id, const KoID & category, const QString & entry)
    : KisBaseProcessor(_id, category, entry),
      m_supportsLevelOfDetail(false)
{
    init(id() + "_filter_bookmarks");
}

KisFilter::~KisFilter()
{
}

void KisFilter::process(KisPaintDeviceSP device,
                        const QRect& applyRect,
                        const KisFilterConfigurationSP config,
                        KoUpdater* progressUpdater) const
{
    process(device, device, KisSelectionSP(), applyRect, config, progressUpdater);
}

void KisFilter::process(const KisPaintDeviceSP src,
                        KisPaintDeviceSP dst,
                        KisSelectionSP selection,
                        const QRect& applyRect,
                        const KisFilterConfigurationSP config,
                        KoUpdater* progressUpdater ) const
{
    if (applyRect.isEmpty()) return;
    QRect needRect = neededRect(applyRect, config, src->defaultBounds()->currentLevelOfDetail());

    KisPaintDeviceSP temporary;
    KisTransaction *transaction = 0;

    bool weirdDstColorSpace =
        dst->colorSpace() != dst->compositionSourceColorSpace() &&
        *dst->colorSpace() != *dst->compositionSourceColorSpace();

    if(src == dst && !selection && !weirdDstColorSpace) {
        temporary = src;
    }
    else {
        temporary = dst->createCompositionSourceDevice(src, needRect);
        transaction = new KisTransaction(temporary);
    }

    try {
        QScopedPointer<KoUpdater> fakeUpdater;

        if (!progressUpdater) {
            // TODO: remove dependency on KoUpdater, depend on KoProgressProxy,
            //       it is more lightweight
            fakeUpdater.reset(new KoDummyUpdater());
            progressUpdater = fakeUpdater.data();
        }

        processImpl(temporary, applyRect, config, progressUpdater);
    }
    catch (const std::bad_alloc&) {
        warnKrita << "Filter" << name() << "failed to allocate enough memory to run.";
    }


    if(transaction) {
        delete transaction;
        KisPainter::copyAreaOptimized(applyRect.topLeft(), temporary, dst, applyRect, selection);
    }
}

QRect KisFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP c, int lod) const
{
    Q_UNUSED(c);
    Q_UNUSED(lod);
    return rect;
}

QRect KisFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP c, int lod) const
{
    Q_UNUSED(c);
    Q_UNUSED(lod);
    return rect;
}

bool KisFilter::supportsLevelOfDetail(const KisFilterConfigurationSP config, int lod) const
{
    Q_UNUSED(config);
    Q_UNUSED(lod);
    return m_supportsLevelOfDetail;
}

void KisFilter::setSupportsLevelOfDetail(bool value)
{
    m_supportsLevelOfDetail = value;
}

bool KisFilter::needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const
{
    Q_UNUSED(config);
    Q_UNUSED(cs);

    return false;
}
