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

#include "kritapaintop_export.h"
#include <functional>

class KisBrush;
typedef QSharedPointer<KisBrush> KisBrushSP;

class KisColorSource;
class KisPressureSharpnessOption;
class KisTextureProperties;


namespace KisDabCacheUtils
{

struct PAINTOP_EXPORT DabRenderingResources
{
    DabRenderingResources();
    virtual ~DabRenderingResources();

    virtual void syncResourcesToSeqNo(int seqNo, const KisPaintInformation &info);

    KisBrushSP brush;
    QScopedPointer<KisColorSource> colorSource;

    QScopedPointer<KisPressureSharpnessOption> sharpnessOption;
    QScopedPointer<KisTextureProperties> textureOption;

    KisPaintDeviceSP colorSourceDevice;

private:
    DabRenderingResources(const DabRenderingResources &rhs) = delete;
};

typedef std::function<DabRenderingResources*()> ResourcesFactory;

struct PAINTOP_EXPORT DabRequestInfo
{
    DabRequestInfo(const KoColor &_color,
                   const QPointF &_cursorPoint,
                   const KisDabShape &_shape,
                   const KisPaintInformation &_info,
                   qreal _softnessFactor)
        : color(_color),
          cursorPoint(_cursorPoint),
          shape(_shape),
          info(_info),
          softnessFactor(_softnessFactor)
    {
    }

    const KoColor &color;
    const QPointF &cursorPoint;
    const KisDabShape &shape;
    const KisPaintInformation &info;
    const qreal softnessFactor;

private:
    DabRequestInfo(const DabRequestInfo &rhs);
};

struct PAINTOP_EXPORT DabGenerationInfo
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

PAINTOP_EXPORT QRect correctDabRectWhenFetchedFromCache(const QRect &dabRect,
                                                        const QSize &realDabSize);

PAINTOP_EXPORT void generateDab(const DabGenerationInfo &di,
                                DabRenderingResources *resources,
                                KisFixedPaintDeviceSP *dab);

PAINTOP_EXPORT void postProcessDab(KisFixedPaintDeviceSP dab,
                                   const QPoint &dabTopLeft,
                                   const KisPaintInformation& info,
                                   DabRenderingResources *resources);

}

template<class T> class QSharedPointer;
class KisDabRenderingJob;
typedef QSharedPointer<KisDabRenderingJob> KisDabRenderingJobSP;

#endif // KISDABCACHEUTILS_H
