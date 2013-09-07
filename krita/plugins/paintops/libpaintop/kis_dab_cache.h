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
struct MirrorProperties;


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
                                   double subPixelX = 0.0, double subPixelY = 0.0,
                                   qreal softnessFactor = 1.0);

    KisFixedPaintDeviceSP fetchDab(const KoColorSpace *cs,
                                   const KisColorSource *colorSource,
                                   double scaleX, double scaleY,
                                   double angle,
                                   const KisPaintInformation& info,
                                   double subPixelX = 0.0, double subPixelY = 0.0,
                                   qreal softnessFactor = 1.0);

private:
    struct SavedDabParameters;

private:
    inline SavedDabParameters getDabParameters(const KoColor& color,
                                               double scaleX, double scaleY,
                                               double angle,
                                               const KisPaintInformation& info,
                                               double subPixelX, double subPixelY,
                                               qreal softnessFactor,
                                               MirrorProperties mirrorProperties);

    inline KisFixedPaintDeviceSP tryFetchFromCache(const KisColorSource *colorSource,
                                                   const KoColor& color,
                                                   double scaleX, double scaleY,
                                                   double angle,
                                                   const KisPaintInformation& info,
                                                   double subPixelX, double subPixelY,
                                                   qreal softnessFactor,
                                                   MirrorProperties mirrorProperties);

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

    struct Private;
    Private * const m_d;

};

#endif /* __KIS_DAB_CACHE_H */
