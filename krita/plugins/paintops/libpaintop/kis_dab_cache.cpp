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

#include <kis_pressure_mirror_option.h>
#include <kis_pressure_sharpness_option.h>
#include <kis_texture_option.h>
#include <kis_precision_option.h>


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

KisDabCache::KisDabCache(KisBrushSP brush)
    : m_brush(brush),
      m_mirrorOption(0),
      m_sharpnessOption(0),
      m_textureOption(0),
      m_precisionOption(0),
      m_cachedDabParameters(new SavedDabParameters)
{
    INIT_HIT_RATE_VARS();
}

KisDabCache::~KisDabCache()
{
    PRINT_HIT_RATE();
    delete m_cachedDabParameters;
}

void KisDabCache::setMirrorPostprocessing(KisPressureMirrorOption *option)
{
    m_mirrorOption = option;
}

void KisDabCache::setSharpnessPostprocessing(KisPressureSharpnessOption *option)
{
    m_sharpnessOption = option;
}

void KisDabCache::setTexturePostprocessing(KisTextureProperties *option)
{
    m_textureOption = option;
}

void KisDabCache::setPrecisionOption(KisPrecisionOption *option)
{
    m_precisionOption = option;
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
    params.width = m_brush->maskWidth(scaleX, angle, info);
    params.height = m_brush->maskHeight(scaleY, angle, info);
    params.subPixelX = subPixelX;
    params.subPixelY = subPixelY;
    params.softnessFactor = softnessFactor;
    params.index = m_brush->brushIndex(info);

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
    return (m_mirrorOption &&
            m_mirrorOption->isChecked() &&
            (m_mirrorOption->isHorizontalMirrorEnabled() ||
             m_mirrorOption->isVerticalMirrorEnabled())) ||
        (m_mirrorOption && m_textureOption->enabled);
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
        COUNT_MISS();
        return 0;
    }

    KoColor newColor = colorSource ? colorSource->uniformColor() : color;

    SavedDabParameters newParams = getDabParameters(newColor,
                                                    scaleX, scaleY,
                                                    angle, info,
                                                    subPixelX, subPixelY,
                                                    softnessFactor);

    int precisionLevel = m_precisionOption ? m_precisionOption->precisionLevel() - 1 : 3;

    if (!newParams.compare(*m_cachedDabParameters, precisionLevel)) {
        COUNT_MISS();
        return 0;
    }

    if (needSeparateOriginal()) {
        *m_dab = *m_dabOriginal;
        postProcessDab(m_dab, info);
        COUNT_HALF_HIT();
    } else {
        COUNT_HIT();
    }

    m_brush->notifyCachedDabPainted();
    return m_dab;
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
    if (!m_dab || !(*m_dab->colorSpace() == *cs)) {
        m_dab = new KisFixedPaintDevice(cs);
    } else {
        KisFixedPaintDeviceSP cachedDab =
            tryFetchFromCache(colorSource, color, scaleX, scaleY, angle,
                              info, subPixelX, subPixelY, softnessFactor);
        if (cachedDab) return cachedDab;
    }

    if (m_brush->brushType() == IMAGE || m_brush->brushType() == PIPE_IMAGE) {
        m_dab = m_brush->paintDevice(cs, scaleX, angle, info,
                                     subPixelX, subPixelY);
    }
    else {
        if (!colorSource) {
            KoColor paintColor = color;
            paintColor.convertTo(cs);

            *m_cachedDabParameters = getDabParameters(paintColor,
                                                      scaleX, scaleY,
                                                      angle, info,
                                                      subPixelX, subPixelY,
                                                      softnessFactor);

            m_brush->mask(m_dab, paintColor, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);

        } else if (colorSource->isUniformColor()) {
            KoColor paintColor = colorSource->uniformColor();
            paintColor.convertTo(cs);

            *m_cachedDabParameters = getDabParameters(paintColor,
                                                      scaleX, scaleY,
                                                      angle, info,
                                                      subPixelX, subPixelY,
                                                      softnessFactor);

            m_brush->mask(m_dab, paintColor, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);
        } else {
            if (!m_colorSourceDevice || !(*cs == *m_colorSourceDevice->colorSpace())) {
                m_colorSourceDevice = new KisPaintDevice(cs);
            } else {
                m_colorSourceDevice->clear();
            }

            QRect maskRect(0, 0, m_brush->maskWidth(scaleX, angle, info), m_brush->maskHeight(scaleY, angle, info));
            colorSource->colorize(m_colorSourceDevice, maskRect, info.pos().toPoint());
            delete m_colorSourceDevice->convertTo(cs);

            m_brush->mask(m_dab, m_colorSourceDevice, scaleX, scaleY, angle,
                          info, subPixelX, subPixelY, softnessFactor);
        }
    }

    if (needSeparateOriginal()) {
        if (!m_dabOriginal || !(*cs == *m_dabOriginal->colorSpace())) {
            m_dabOriginal = new KisFixedPaintDevice(cs);
        }

        *m_dabOriginal = *m_dab;
    }

    postProcessDab(m_dab, info);

    return m_dab;
}

void KisDabCache::postProcessDab(KisFixedPaintDeviceSP dab,
                                 const KisPaintInformation& info)
{
    if (m_mirrorOption) {
        MirrorProperties mirror = m_mirrorOption->apply(info);
        dab->mirror(mirror.horizontalMirror, mirror.verticalMirror);
    }

    if (m_sharpnessOption) {
        m_sharpnessOption->applyThreshold(dab);
    }

    if (m_textureOption) {
        m_textureOption->apply(dab, info.pos().toPoint());
    }
}
