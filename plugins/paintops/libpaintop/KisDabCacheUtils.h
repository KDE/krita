/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                   qreal _softnessFactor,
                   qreal _lightnessStrength = 1.0)
        : color(_color),
          cursorPoint(_cursorPoint),
          shape(_shape),
          info(_info),
          softnessFactor(_softnessFactor),
          lightnessStrength(_lightnessStrength)
    {
    }

    const KoColor &color;
    const QPointF &cursorPoint;
    const KisDabShape &shape;
    const KisPaintInformation &info;
    const qreal softnessFactor;
    const qreal lightnessStrength;

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
    qreal lightnessStrength = 1.0;

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
