/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISDABCACHEUTILS_H
#define KISDABCACHEUTILS_H

#include <QRect>
#include <QSize>

#include "kis_types.h"

#include <kis_pressure_mirror_option.h>
#include "kis_dab_shape.h"

class KisBrush;
typedef KisSharedPtr<KisBrush> KisBrushSP;

class KisColorSource;
class KisPressureSharpnessOption;
class KisTextureProperties;


namespace KisDabCacheUtils
{

struct DabRenderingResources
{
    KisBrushSP brush;
    const KisColorSource *colorSource;

    KisPressureSharpnessOption *sharpnessOption;
    KisTextureProperties *textureOption;

    KisPaintDeviceSP colorSourceDevice;
};

struct DabGenerationInfo
{
    MirrorProperties mirrorProperties;
    KisDabShape shape;
    QRect dstDabRect;
    QPointF subPixel;
    bool solidColorFill = true;
    KoColor paintColor;
    KisPaintInformation info;
    qreal softnessFactor = 1.0;

    bool needsPostprocessing = false;
};

QRect correctDabRectWhenFetchedFromCache(const QRect &dabRect,
                                         const QSize &realDabSize);

void generateDab(const DabGenerationInfo &di,
                 DabRenderingResources *resources,
                 KisFixedPaintDeviceSP *dab);

void postProcessDab(KisFixedPaintDeviceSP dab,
                    const QPoint &dabTopLeft,
                    const KisPaintInformation& info,
                    DabRenderingResources *resources);

}

#endif // KISDABCACHEUTILS_H
