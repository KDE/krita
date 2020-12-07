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

KisGradientMapFilter::KisGradientMapFilter()
    : KisFilter(id(), FiltersCategoryMapId, i18n("&Gradient Map..."))
{
    setSupportsPainting(true);
}

class NearestCachedGradient
{
public:
    NearestCachedGradient(const KoStopGradientSP gradient, qint32 steps, const KoColorSpace *cs);

    /// gets the color data at position 0 <= t <= 1
    const quint8* cachedAt(qreal t) const;

private:
    const qint32 m_max;
    QVector<KoColor> m_colors;
    const KoColor m_black;
};

NearestCachedGradient::NearestCachedGradient(const KoStopGradientSP gradient, qint32 steps, const KoColorSpace *cs)
    : m_max(steps - 1)
    , m_black(KoColor(cs))
{
    for (qint32 i = 0; i < steps; i++) {
        qreal t = static_cast<qreal>(i) / m_max;
        KoGradientStop leftStop, rightStop;
        if (!gradient->stopsAt(leftStop, rightStop, t)) {
            m_colors << m_black;
        } else {
            if (std::abs(t - leftStop.position) < std::abs(t - rightStop.position)) {
                m_colors << leftStop.color.convertedTo(cs);
            } else {
                m_colors << rightStop.color.convertedTo(cs);
            }
        }
    }
}

const quint8* NearestCachedGradient::cachedAt(qreal t) const
{
    qint32 tInt = t * m_max + 0.5;
    if (m_colors.size() > tInt) {
        return m_colors[tInt].data();
    } else {
        return m_black.data();
    }
}

class DitherCachedGradient
{
public:
    struct CachedEntry
    {
        KoColor leftStop;
        KoColor rightStop;
        qreal localT;
    };

    DitherCachedGradient(const KoStopGradientSP gradient, qint32 steps, const KoColorSpace *cs);

    /// gets the color data at position 0 <= t <= 1
    const CachedEntry& cachedAt(qreal t) const;

private:
    const qint32 m_max;
    QVector<CachedEntry> m_cachedEntries;
    const CachedEntry m_nullEntry;
};

DitherCachedGradient::DitherCachedGradient(const KoStopGradientSP gradient, qint32 steps, const KoColorSpace *cs)
    : m_max(steps - 1)
    , m_nullEntry(CachedEntry{KoColor(cs), KoColor(cs), 0.0})
{
    for (qint32 i = 0; i < steps; i++) {
        qreal t = static_cast<qreal>(i) / m_max;
        KoGradientStop leftStop, rightStop;
        if (!gradient->stopsAt(leftStop, rightStop, t)) {
            m_cachedEntries << m_nullEntry;
        } else {
            const qreal localT = (t - leftStop.position) / (rightStop.position - leftStop.position);
            m_cachedEntries << CachedEntry{leftStop.color.convertedTo(cs), rightStop.color.convertedTo(cs), localT};
        }
    }
}

const DitherCachedGradient::CachedEntry& DitherCachedGradient::cachedAt(qreal t) const
{
    qint32 tInt = t * m_max + 0.5;
    if (m_cachedEntries.size() > tInt) {
        return m_cachedEntries[tInt];
    } else {
        return m_nullEntry;
    }
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
    NearestColorModePolicy(const NearestCachedGradient *cachedGradient);

    const quint8* colorAt(qreal t, int x, int y) const;

private:
    const NearestCachedGradient *m_cachedGradient;
};

NearestColorModePolicy::NearestColorModePolicy(const NearestCachedGradient *cachedGradient)
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
    DitherColorModePolicy(const DitherCachedGradient *cachedGradient, KisDitherUtil *ditherUtil);

    const quint8* colorAt(qreal t, int x, int y) const;

private:
    const DitherCachedGradient *m_cachedGradient;
    KisDitherUtil *m_ditherUtil;
};

DitherColorModePolicy::DitherColorModePolicy(const DitherCachedGradient *cachedGradient, KisDitherUtil *ditherUtil)
    : m_cachedGradient(cachedGradient)
    , m_ditherUtil(ditherUtil)
{}

const quint8* DitherColorModePolicy::colorAt(qreal t, int x, int y) const
{
    const DitherCachedGradient::CachedEntry &cachedEntry = m_cachedGradient->cachedAt(t);
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

    KoStopGradientSP gradient = filterConfig->gradient();
    const ColorMode colorMode = ColorMode(filterConfig->getInt("colorMode"));
    const KoColorSpace *colorSpace = device->colorSpace();

    if (colorMode == ColorMode::Blend) {
        KoCachedGradient cachedGradient(gradient, qMax(device->extent().width(), device->extent().height()), colorSpace);
        BlendColorModePolicy colorModePolicy(&cachedGradient);
        processImpl(device, applyRect, config, progressUpdater, colorModePolicy);
    } else if (colorMode == ColorMode::Nearest) {
        NearestCachedGradient cachedGradient(gradient, qMax(device->extent().width(), device->extent().height()), colorSpace);
        NearestColorModePolicy colorModePolicy(&cachedGradient);
        processImpl(device, applyRect, config, progressUpdater, colorModePolicy);
    } else /* if colorMode == ColorMode::Dither */ {
        KisDitherUtil ditherUtil;
        DitherCachedGradient cachedGradient(gradient, qMax(device->extent().width(), device->extent().height()), colorSpace);
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
