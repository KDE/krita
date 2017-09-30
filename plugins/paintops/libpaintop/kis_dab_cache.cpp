/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_dab_cache.h"

#include <KoColor.h>
#include "kis_paint_device.h"
#include "kis_brush.h"
#include <kis_fixed_paint_device.h>
#include "kis_color_source.h"
#include "kis_pressure_sharpness_option.h"
#include "kis_texture_option.h"

#include <kundo2command.h>

struct KisDabCache::Private {

    Private(KisBrushSP brush)
        : brush(brush)
    {}

    KisFixedPaintDeviceSP dab;
    KisFixedPaintDeviceSP dabOriginal;

    KisBrushSP brush;
    KisPaintDeviceSP colorSourceDevice;

    KisPressureSharpnessOption *sharpnessOption = 0;
    KisTextureProperties *textureOption = 0;
};



KisDabCache::KisDabCache(KisBrushSP brush)
    : m_d(new Private(brush))
{
}

KisDabCache::~KisDabCache()
{
    delete m_d;
}

void KisDabCache::setSharpnessPostprocessing(KisPressureSharpnessOption *option)
{
    m_d->sharpnessOption = option;
}

void KisDabCache::setTexturePostprocessing(KisTextureProperties *option)
{
    m_d->textureOption = option;
}

bool KisDabCache::needSeparateOriginal() const
{
    return KisDabCacheBase::needSeparateOriginal(m_d->textureOption, m_d->sharpnessOption);
}


KisFixedPaintDeviceSP KisDabCache::fetchDab(const KoColorSpace *cs,
        KisColorSource *colorSource,
        const QPointF &cursorPoint,
        KisDabShape const& shape,
        const KisPaintInformation& info,
        qreal softnessFactor,
        QRect *dstDabRect)
{
    return fetchDabCommon(cs, colorSource, KoColor(),
                          cursorPoint,
                          shape,
                          info,
                          softnessFactor,
                          dstDabRect);
}

KisFixedPaintDeviceSP KisDabCache::fetchDab(const KoColorSpace *cs,
        const KoColor& color,
        const QPointF &cursorPoint,
        KisDabShape const& shape,
        const KisPaintInformation& info,
        qreal softnessFactor,
        QRect *dstDabRect)
{
    return fetchDabCommon(cs, 0, color,
                          cursorPoint,
                          shape,
                          info,
                          softnessFactor,
                          dstDabRect);
}

inline
KisFixedPaintDeviceSP KisDabCache::fetchFromCache(KisDabCacheUtils::DabRenderingResources *resources,
                                                  const KisPaintInformation& info,
                                                  QRect *dstDabRect)
{
    if (needSeparateOriginal()) {
        *m_d->dab = *m_d->dabOriginal;
        *dstDabRect = KisDabCacheUtils::correctDabRectWhenFetchedFromCache(*dstDabRect, m_d->dab->bounds().size());
        KisDabCacheUtils::postProcessDab(m_d->dab, dstDabRect->topLeft(), info, resources);
    }
    else {
        *dstDabRect = KisDabCacheUtils::correctDabRectWhenFetchedFromCache(*dstDabRect, m_d->dab->bounds().size());
    }

    resources->brush->notifyCachedDabPainted(info);
    return m_d->dab;
}

/**
 * A special hack class that allows creation of temporary object with resources
 * without taking ownershop over the option classes
 */
struct TemporaryResourcesWithoutOwning : public KisDabCacheUtils::DabRenderingResources
{
    ~TemporaryResourcesWithoutOwning() override {
        // we do not own these resources, so just
        // release them before destruction
        colorSource.take();
        sharpnessOption.take();
        textureOption.take();
    }
};

inline
KisFixedPaintDeviceSP KisDabCache::fetchDabCommon(const KoColorSpace *cs,
        KisColorSource *colorSource,
        const KoColor& color,
        const QPointF &cursorPoint,
        KisDabShape shape,
        const KisPaintInformation& info,
        qreal softnessFactor,
        QRect *dstDabRect)
{
    Q_ASSERT(dstDabRect);

    bool hasDabInCache = true;

    if (!m_d->dab || *m_d->dab->colorSpace() != *cs) {
        m_d->dab = new KisFixedPaintDevice(cs);
        hasDabInCache = false;
    }

    using namespace KisDabCacheUtils;

    // 1. Calculate new dab parameters and whether we can reuse the cache

    TemporaryResourcesWithoutOwning resources;
    resources.brush = m_d->brush;
    resources.colorSourceDevice = m_d->colorSourceDevice;

    // NOTE: we use a special subclass of resources that will NOT
    //       delete options on destruction!
    resources.colorSource.reset(colorSource);
    resources.sharpnessOption.reset(m_d->sharpnessOption);
    resources.textureOption.reset(m_d->textureOption);


    DabGenerationInfo di;
    bool shouldUseCache = false;

    fetchDabGenerationInfo(hasDabInCache,
                           &resources,
                           DabRequestInfo(
                               color,
                               cursorPoint,
                               shape,
                               info,
                               softnessFactor),
                           &di,
                           &shouldUseCache);

    *dstDabRect = di.dstDabRect;


    // 2. Try return a saved dab from the cache

    if (shouldUseCache) {
        return fetchFromCache(&resources, info, dstDabRect);
    }

    // 3. Generate new dab

    generateDab(di, &resources, &m_d->dab);

    // 4. Do postprocessing
    if (di.needsPostprocessing) {
        if (!m_d->dabOriginal || *cs != *m_d->dabOriginal->colorSpace()) {
            m_d->dabOriginal = new KisFixedPaintDevice(cs);
        }

        *m_d->dabOriginal = *m_d->dab;

        postProcessDab(m_d->dab, di.dstDabRect.topLeft(), info, &resources);
    }

    return m_d->dab;
}
