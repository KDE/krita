/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoColorSpace.h>
#include <KoColor.h>
#include <kis_paint_device.h>
#include <kis_global.h>
#include <kis_types.h>
#include <filter/kis_filter_category_ids.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoColorSet.h>
#include <KisDitherUtil.h>
#include <KisGlobalResourcesInterface.h>
#include <KisSequentialIteratorProgress.h>
#include <KoUpdater.h>
#include <KoCachedGradient.h>

#include "KisGradientMapFilter.h"
#include "KisGradientMapFilterConfigWidget.h"
#include "KisGradientMapFilterConfiguration.h"
#include "KisGradientMapFilterNearestCachedGradient.h"
#include "KisGradientMapFilterDitherCachedGradient.h"

KisGradientMapFilter::KisGradientMapFilter()
    : KisFilter(id(), FiltersCategoryMapId, i18n("&Gradient Map..."))
{
    setSupportsPainting(true);
}

class BlendColorModePolicy
{
public:
    BlendColorModePolicy(const KoCachedGradient *cachedGradient);

    const quint8* colorAt(qreal t, int x, int y) const;

private:
    const KoCachedGradient *m_cachedGradient;
};

BlendColorModePolicy::BlendColorModePolicy(const KoCachedGradient *cachedGradient)
    : m_cachedGradient(cachedGradient)
{}

const quint8* BlendColorModePolicy::colorAt(qreal t, int x, int y) const
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    return m_cachedGradient->cachedAt(t);
}

class NearestColorModePolicy
{
public:
    NearestColorModePolicy(const KisGradientMapFilterNearestCachedGradient *cachedGradient);

    const quint8* colorAt(qreal t, int x, int y) const;

private:
    const KisGradientMapFilterNearestCachedGradient *m_cachedGradient;
};

NearestColorModePolicy::NearestColorModePolicy(const KisGradientMapFilterNearestCachedGradient *cachedGradient)
    : m_cachedGradient(cachedGradient)
{}

const quint8* NearestColorModePolicy::colorAt(qreal t, int x, int y) const
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    return m_cachedGradient->cachedAt(t);
}

class DitherColorModePolicy
{
public:
    DitherColorModePolicy(const KisGradientMapFilterDitherCachedGradient *cachedGradient, KisDitherUtil *ditherUtil);

    const quint8* colorAt(qreal t, int x, int y) const;

private:
    const KisGradientMapFilterDitherCachedGradient *m_cachedGradient;
    KisDitherUtil *m_ditherUtil;
};

DitherColorModePolicy::DitherColorModePolicy(const KisGradientMapFilterDitherCachedGradient *cachedGradient, KisDitherUtil *ditherUtil)
    : m_cachedGradient(cachedGradient)
    , m_ditherUtil(ditherUtil)
{}

const quint8* DitherColorModePolicy::colorAt(qreal t, int x, int y) const
{
    const KisGradientMapFilterDitherCachedGradient::CachedEntry &cachedEntry = m_cachedGradient->cachedAt(t);
    if (cachedEntry.localT < m_ditherUtil->threshold(QPoint(x, y))) {
        return cachedEntry.leftStop.data();
    }
    else {
        return cachedEntry.rightStop.data();
    }
}

void KisGradientMapFilter::processImpl(KisPaintDeviceSP device,
                                       const QRect& applyRect,
                                       const KisFilterConfigurationSP config,
                                       KoUpdater *progressUpdater) const
{
    Q_ASSERT(!device.isNull());

    const KisGradientMapFilterConfiguration *filterConfig =
        dynamic_cast<const KisGradientMapFilterConfiguration*>(config.data());

    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);

    KoAbstractGradientSP gradient = filterConfig->gradient();
    const int colorMode = filterConfig->colorMode();
    const KoColorSpace *colorSpace = device->colorSpace();
    const int cachedGradientSize = qMax(device->extent().width(), device->extent().height());

    if (colorMode == KisGradientMapFilterConfiguration::ColorMode_Blend) {
        KoCachedGradient cachedGradient(gradient, cachedGradientSize, colorSpace);
        BlendColorModePolicy colorModePolicy(&cachedGradient);
        processImpl(device, applyRect, config, progressUpdater, colorModePolicy);
    } else if (colorMode == KisGradientMapFilterConfiguration::ColorMode_Nearest) {
        KisGradientMapFilterNearestCachedGradient cachedGradient(gradient, cachedGradientSize, colorSpace);
        NearestColorModePolicy colorModePolicy(&cachedGradient);
        processImpl(device, applyRect, config, progressUpdater, colorModePolicy);
    } else /* if colorMode == KisGradientMapFilterConfiguration::ColorMode_Dither */ {
        KisDitherUtil ditherUtil;
        KisGradientMapFilterDitherCachedGradient cachedGradient(gradient, cachedGradientSize, colorSpace);
        ditherUtil.setConfiguration(*filterConfig, "dither/");
        DitherColorModePolicy colorModePolicy(&cachedGradient, &ditherUtil);
        processImpl(device, applyRect, config, progressUpdater, colorModePolicy);
    }
}

template <typename ColorModeStrategy>
void KisGradientMapFilter::processImpl(KisPaintDeviceSP device,
                                       const QRect& applyRect,
                                       const KisFilterConfigurationSP config,
                                       KoUpdater *progressUpdater,
                                       const ColorModeStrategy &colorModeStrategy) const
{
    Q_UNUSED(config);
    
    Q_ASSERT(!device.isNull());

    const KoColorSpace *colorSpace = device->colorSpace();
    const int pixelSize = colorSpace->pixelSize();

    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);

    while (it.nextPixel()) {
        const qreal t = static_cast<qreal>(colorSpace->intensity8(it.oldRawData())) / 255;
        const qreal pixelOpacity = colorSpace->opacityF(it.oldRawData());
        const quint8 *color = colorModeStrategy.colorAt(t, it.x(), it.y());
        memcpy(it.rawData(), color, pixelSize);
        colorSpace->setOpacity(it.rawData(), qMin(pixelOpacity, colorSpace->opacityF(color)), 1);
    }
}

KisFilterConfigurationSP KisGradientMapFilter::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisGradientMapFilterConfiguration(resourcesInterface);
}

KisFilterConfigurationSP KisGradientMapFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisGradientMapFilterConfigurationSP config = new KisGradientMapFilterConfiguration(resourcesInterface);
    config->setDefaults();
    return config;
}

KisConfigWidget* KisGradientMapFilter::createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP, bool) const
{
    return new KisGradientMapFilterConfigWidget(parent);
}
