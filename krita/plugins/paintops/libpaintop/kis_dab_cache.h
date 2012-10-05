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

#ifndef __KIS_DAB_CACHE_H
#define __KIS_DAB_CACHE_H

#include "krita_export.h"
#include "kis_brush.h"

class KisColorSource;
class KisPressureSharpnessOption;
class KisTextureProperties;
class KisPressureMirrorOption;
class KisPrecisionOption;

//#define DEBUG_HIT_RATE

#ifdef DEBUG_HIT_RATE
#define DECLARE_HIT_RATE_VARS() int m_hitRate; int m_halfHitRate; int m_missRate
#define INIT_HIT_RATE_VARS() m_hitRate = 0; m_halfHitRate = 0; m_missRate = 0
#define COUNT_HIT() m_hitRate++
#define COUNT_HALF_HIT() m_halfHitRate++
#define COUNT_MISS() m_missRate++
#define PRINT_HIT_RATE() if(m_hitRate + m_halfHitRate + m_missRate > 0) { \
    qDebug() << "Hits:" << m_hitRate                                    \
             << "HalfHits:" << m_halfHitRate                            \
             << "Misses:" << m_missRate                                 \
             << "HitRate:" << qreal(m_hitRate) / (m_hitRate + m_halfHitRate + m_missRate) \
             << "HalfHitRate:" << qreal(m_halfHitRate) / (m_hitRate + m_halfHitRate + m_missRate); \
    }
#else
#define DECLARE_HIT_RATE_VARS()
#define INIT_HIT_RATE_VARS()
#define COUNT_HIT()
#define COUNT_HALF_HIT()
#define COUNT_MISS()
#define PRINT_HIT_RATE()
#endif


class PAINTOP_EXPORT KisDabCache
{
public:
    KisDabCache(KisBrushSP brush);
    ~KisDabCache();

    void setMirrorPostprocessing(KisPressureMirrorOption *option);
    void setSharpnessPostprocessing(KisPressureSharpnessOption *option);
    void setTexturePostprocessing(KisTextureProperties *option);
    void setPrecisionOption(KisPrecisionOption *option);

    bool needSeparateOriginal();

    KisFixedPaintDeviceSP fetchDab(const KoColorSpace *cs,
                                   const KoColor& color,
                                   double scaleX, double scaleY,
                                   double angle,
                                   const KisPaintInformation& info,
                                   double subPixelX, double subPixelY,
                                   qreal softnessFactor);

    KisFixedPaintDeviceSP fetchDab(const KoColorSpace *cs,
                                   const KisColorSource *colorSource,
                                   double scaleX, double scaleY,
                                   double angle,
                                   const KisPaintInformation& info,
                                   double subPixelX, double subPixelY,
                                   qreal softnessFactor);

private:
    struct SavedDabParameters;

private:
    inline SavedDabParameters getDabParameters(const KoColor& color,
                                               double scaleX, double scaleY,
                                               double angle,
                                               const KisPaintInformation& info,
                                               double subPixelX, double subPixelY,
                                               qreal softnessFactor);

    inline KisFixedPaintDeviceSP tryFetchFromCache(const KisColorSource *colorSource,
                                                   const KoColor& color,
                                                   double scaleX, double scaleY,
                                                   double angle,
                                                   const KisPaintInformation& info,
                                                   double subPixelX, double subPixelY,
                                                   qreal softnessFactor);

    inline KisFixedPaintDeviceSP fetchDabCommon(const KoColorSpace *cs,
                                                const KisColorSource *colorSource,
                                                const KoColor& color,
                                                double scaleX, double scaleY,
                                                double angle,
                                                const KisPaintInformation& info,
                                                double subPixelX, double subPixelY,
                                                qreal softnessFactor);

    void postProcessDab(KisFixedPaintDeviceSP dab, const KisPaintInformation& info);

private:
    KisFixedPaintDeviceSP m_dab;
    KisFixedPaintDeviceSP m_dabOriginal;

    KisBrushSP m_brush;
    KisPaintDeviceSP m_colorSourceDevice;

    KisPressureMirrorOption *m_mirrorOption;
    KisPressureSharpnessOption *m_sharpnessOption;
    KisTextureProperties *m_textureOption;
    KisPrecisionOption *m_precisionOption;

    SavedDabParameters *m_cachedDabParameters;

    DECLARE_HIT_RATE_VARS();
};

#endif /* __KIS_DAB_CACHE_H */
