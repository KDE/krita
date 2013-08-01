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
#include "kis_color_source.h"
#include "kis_paint_device.h"
#include "kis_brush.h"
#include <kis_pressure_mirror_option.h>
#include <kis_pressure_sharpness_option.h>
#include <kis_texture_option.h>
#include <kis_precision_option.h>
#include <kis_fixed_paint_device.h>

#include <kundo2command.h>

struct PrecisionValues {
    qreal angle;
    qreal sizeFrac;
    qreal subPixel;
    qreal softnessFactor;
};

const qreal eps = 1e-6;
static PrecisionValues precisionLevels[] =
{
    {M_PI/180, 0.05,   1, 0.01},
    {M_PI/180, 0.01,   1, 0.01},
    {M_PI/180,    0,   1, 0.01},
    {M_PI/180,    0, 0.5, 0.01},
    {eps,         0, eps,  eps}
};

struct KisDabCache::SavedDabParameters {
    KoColor color;
    qreal angle;
    int width;
    int height;
    qreal subPixelX;
    qreal subPixelY;
    qreal softnessFactor;
    int index;

    bool compare(const SavedDabParameters &rhs, int precisionLevel) {
        PrecisionValues &prec = precisionLevels[precisionLevel];

        return color == rhs.color &&
            qAbs(angle - rhs.angle) <= prec.angle &&
            qAbs(width - rhs.width) <= (int)(prec.sizeFrac * width) &&
            qAbs(height - rhs.height) <= (int)(prec.sizeFrac * height) &&
            qAbs(subPixelX - rhs.subPixelX) <= prec.subPixel &&
            qAbs(subPixelY - rhs.subPixelY) <= prec.subPixel &&
            qAbs(softnessFactor - rhs.softnessFactor) <= prec.softnessFactor &&
            index == rhs.index;
    }
};

struct KisDabCache::Private {

    Private(KisBrushSP brush)
        : brush(brush),
          mirrorOption(0),
          sharpnessOption(0),
          textureOption(0),
          precisionOption(0),
          cachedDabParameters(new SavedDabParameters)
    {}
    KisFixedPaintDeviceSP dab;
    KisFixedPaintDeviceSP dabOriginal;

    KisBrushSP brush;
    KisPaintDeviceSP colorSourceDevice;

    KisPressureMirrorOption *mirrorOption;
    KisPressureSharpnessOption *sharpnessOption;
    KisTextureProperties *textureOption;
    KisPrecisionOption *precisionOption;

    SavedDabParameters *cachedDabParameters;
};



KisDabCache::KisDabCache(KisBrushSP brush)
    : m_d(new Private(brush))
{
}

KisDabCache::~KisDabCache()
{
    delete m_d->cachedDabParameters;
    delete m_d;
}

void KisDabCache::setMirrorPostprocessing(KisPressureMirrorOption *option)
{
    m_d->mirrorOption = option;
}

void KisDabCache::setSharpnessPostprocessing(KisPressureSharpnessOption *option)
{
    m_d->sharpnessOption = option;
}

void KisDabCache::setTexturePostprocessing(KisTextureProperties *option)
{
    m_d->textureOption = option;
}

void KisDabCache::setPrecisionOption(KisPrecisionOption *option)
{
    m_d->precisionOption = option;
}

inline KisDabCache::SavedDabParameters
KisDabCache::getDabParameters(const KoColor& color,
                              double scaleX, double scaleY,
                              double angle,
                              const KisPaintInformation& info,
                              double subPixelX, double subPixelY,
                              qreal softnessFactor)
{
    SavedDabParameters params;

    params.color = color;
    params.angle = angle;
    params.width = m_d->brush->maskWidth(scaleX, angle, info);
    params.height = m_d->brush->maskHeight(scaleY, angle, info);
    params.subPixelX = subPixelX;
    params.subPixelY = subPixelY;
    params.softnessFactor = softnessFactor;
    params.index = m_d->brush->brushIndex(info);

    return params;
}

KisFixedPaintDeviceSP KisDabCache::fetchDab(const KoColorSpace *cs,
                                            const KoColor& color,
                                            double scaleX, double scaleY,
                                            double angle,
                                            const KisPaintInformation& info,
                                            double subPixelX, double subPixelY,
                                            qreal softnessFactor)
{
    return fetchDabCommon(cs, 0, color, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);
}

KisFixedPaintDeviceSP KisDabCache::fetchDab(const KoColorSpace *cs,
                                            const KisColorSource *colorSource,
                                            double scaleX, double scaleY,
                                            double angle,
                                            const KisPaintInformation& info,
                                            double subPixelX, double subPixelY,
                                            qreal softnessFactor)
{
    return fetchDabCommon(cs, colorSource, KoColor(), scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);
}

bool KisDabCache::needSeparateOriginal()
{
    return (m_d->mirrorOption &&
            m_d->mirrorOption->isChecked() &&
            (m_d->mirrorOption->isHorizontalMirrorEnabled() ||
             m_d->mirrorOption->isVerticalMirrorEnabled())) ||
        (m_d->textureOption && m_d->textureOption->enabled);
}

inline
KisFixedPaintDeviceSP KisDabCache::tryFetchFromCache(const KisColorSource *colorSource,
                                                     const KoColor& color,
                                                     double scaleX, double scaleY,
                                                     double angle,
                                                     const KisPaintInformation& info,
                                                     double subPixelX, double subPixelY,
                                                     qreal softnessFactor)
{
    if (colorSource && !colorSource->isUniformColor()) {
        return 0;
    }

    KoColor newColor = colorSource ? colorSource->uniformColor() : color;

    SavedDabParameters newParams = getDabParameters(newColor,
                                                    scaleX, scaleY,
                                                    angle, info,
                                                    subPixelX, subPixelY,
                                                    softnessFactor);

    int precisionLevel = m_d->precisionOption ? m_d->precisionOption->precisionLevel() - 1 : 3;

    if (!newParams.compare(*m_d->cachedDabParameters, precisionLevel)) {
        return 0;
    }

    if (needSeparateOriginal()) {
        *m_d->dab = *m_d->dabOriginal;
        postProcessDab(m_d->dab, info);
    }

    m_d->brush->notifyCachedDabPainted();
    return m_d->dab;
}

inline
KisFixedPaintDeviceSP KisDabCache::fetchDabCommon(const KoColorSpace *cs,
                                                  const KisColorSource *colorSource,
                                                  const KoColor& color,
                                                  double scaleX, double scaleY,
                                                  double angle,
                                                  const KisPaintInformation& info,
                                                  double subPixelX, double subPixelY,
                                                  qreal softnessFactor)
{
    if (!m_d->dab || !(*m_d->dab->colorSpace() == *cs)) {
        m_d->dab = new KisFixedPaintDevice(cs);
    } else {
        KisFixedPaintDeviceSP cachedDab =
            tryFetchFromCache(colorSource, color, scaleX, scaleY, angle,
                              info, subPixelX, subPixelY, softnessFactor);
        if (cachedDab) return cachedDab;
    }

    if (m_d->brush->brushType() == IMAGE || m_d->brush->brushType() == PIPE_IMAGE) {
        m_d->dab = m_d->brush->paintDevice(cs, scaleX, angle, info,
                                     subPixelX, subPixelY);
    }
    else {
        if (!colorSource) {
            KoColor paintColor = color;
            paintColor.convertTo(cs);

            *m_d->cachedDabParameters = getDabParameters(paintColor,
                                                      scaleX, scaleY,
                                                      angle, info,
                                                      subPixelX, subPixelY,
                                                      softnessFactor);

            m_d->brush->mask(m_d->dab, paintColor, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);

        } else if (colorSource->isUniformColor()) {
            KoColor paintColor = colorSource->uniformColor();
            paintColor.convertTo(cs);

            *m_d->cachedDabParameters = getDabParameters(paintColor,
                                                      scaleX, scaleY,
                                                      angle, info,
                                                      subPixelX, subPixelY,
                                                      softnessFactor);

            m_d->brush->mask(m_d->dab, paintColor, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);
        } else {
            if (!m_d->colorSourceDevice || !(*cs == *m_d->colorSourceDevice->colorSpace())) {
                m_d->colorSourceDevice = new KisPaintDevice(cs);
            } else {
                m_d->colorSourceDevice->clear();
            }

            QRect maskRect(0, 0, m_d->brush->maskWidth(scaleX, angle, info), m_d->brush->maskHeight(scaleY, angle, info));
            colorSource->colorize(m_d->colorSourceDevice, maskRect, info.pos().toPoint());
            delete m_d->colorSourceDevice->convertTo(cs);

            m_d->brush->mask(m_d->dab, m_d->colorSourceDevice, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);
        }
    }

    if (needSeparateOriginal()) {
        if (!m_d->dabOriginal || !(*cs == *m_d->dabOriginal->colorSpace())) {
            m_d->dabOriginal = new KisFixedPaintDevice(cs);
        }

        *m_d->dabOriginal = *m_d->dab;
    }

    postProcessDab(m_d->dab, info);

    return m_d->dab;
}

void KisDabCache::postProcessDab(KisFixedPaintDeviceSP dab,
                                 const KisPaintInformation& info)
{
    if (m_d->mirrorOption) {
        MirrorProperties mirror = m_d->mirrorOption->apply(info);
        dab->mirror(mirror.horizontalMirror, mirror.verticalMirror);
    }

    if (m_d->sharpnessOption) {
        m_d->sharpnessOption->applyThreshold(dab);
    }

    if (m_d->textureOption) {
        m_d->textureOption->apply(dab, info.pos().toPoint(), info);
    }
}
