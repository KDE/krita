/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisResetTransparentFilter.h"

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoUpdater.h>
#include <KisSequentialIteratorProgress.h>

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>

K_PLUGIN_FACTORY_WITH_JSON(ResetTransparentFactory, "kritaresettransparent.json", registerPlugin<ResetTransparent>();)

ResetTransparent::ResetTransparent(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisResetTransparentFilter()));
}

ResetTransparent::~ResetTransparent()
{
}

KisResetTransparentFilter::KisResetTransparentFilter()
    : KisFilter(id(), FiltersCategoryOtherId, i18n("Reset Transparent"))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(false);
    setSupportsThreading(true);
    setSupportsLevelOfDetail(false);
    setSupportsAdjustmentLayers(false);
    setShowConfigurationWidget(false);
}

bool KisResetTransparentFilter::needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const
{
    Q_UNUSED(config);
    Q_UNUSED(cs);
    return true;
}

void KisResetTransparentFilter::processImpl(KisPaintDeviceSP device,
                                            const QRect &applyRect,
                                            const KisFilterConfigurationSP config,
                                            KoUpdater *progressUpdater) const
{
    Q_UNUSED(config);

    const KoColorSpace *cs = device->colorSpace();
    KoColor transparent = KoColor::createTransparent(cs);
    const int pixelSize = cs->pixelSize();

    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);
    while (it.nextPixel()) {
        if (cs->opacityU8(it.oldRawData()) == OPACITY_TRANSPARENT_U8) {
            memcpy(it.rawData(), transparent.data(), pixelSize);
        }
    }
}

#include "KisResetTransparentFilter.moc"
