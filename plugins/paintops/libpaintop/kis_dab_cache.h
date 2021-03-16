/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DAB_CACHE_H
#define __KIS_DAB_CACHE_H

#include "kritapaintop_export.h"

#include "kis_dab_cache_base.h"

#include "kis_brush.h"

class KisColorSource;
class KisPressureSharpnessOption;
class KisTextureProperties;
class KisPressureMirrorOption;
class KisPrecisionOption;
struct MirrorProperties;


/**
 * @brief The KisDabCache class provides caching for dabs into the brush paintop
 *
 *  This class adds caching of the dabs to the paintop system of Krita.
 *  Such cache makes the execution of the benchmarks up to 2 times faster.
 *  Subjectively, the real painting becomes much faster, especially with
 *  huge brushes. Artists report up to 20% speed gain while painting.
 *
 *  Of course, such caching makes the painting a bit less precise: we need
 *  to tolerate subpixel differences to allow the cache to work. Sometimes
 *  small difference in the size of a dab can also be acceptable. That is
 *  why I introduced levels of precision. They are graded from 1 to 5: from
 *  the fastest and less precise to the slowest, but with the best quality.
 *  You can see the slider in the paintop settings dialog. The ToolTip text
 *  explains which features of the brush are sacrificed on each precision
 *  level.
 *
 *  The texturing and mirroring problems are solved.
 */
class PAINTOP_EXPORT KisDabCache : public KisDabCacheBase
{
public:
    KisDabCache(KisBrushSP brush);
    ~KisDabCache();

    KisFixedPaintDeviceSP fetchDab(const KoColorSpace *cs,
                                   KisColorSource *colorSource,
                                   const QPointF &cursorPoint,
                                   KisDabShape const&,
                                   const KisPaintInformation& info,
                                   qreal softnessFactor,
                                   QRect *dstDabRect,
                                   qreal lightnessStrength = 1.0);

    KisFixedPaintDeviceSP fetchDab(const KoColorSpace *cs,
                                   const KoColor& color,
                                   const QPointF &cursorPoint,
                                   KisDabShape const&,
                                   const KisPaintInformation& info,
                                   qreal softnessFactor,
                                   QRect *dstDabRect,
                                   qreal lightnessStrength = 1.0);

    KisFixedPaintDeviceSP fetchNormalizedImageDab(const KoColorSpace *cs,
                                                  const QPointF &cursorPoint,
                                                  KisDabShape const& shape,
                                                  const KisPaintInformation& info,
                                                  qreal softnessFactor,
                                                  QRect *dstDabRect);


    void setSharpnessPostprocessing(KisPressureSharpnessOption *option);
    void setTexturePostprocessing(KisTextureProperties *option);

    bool needSeparateOriginal() const;

private:

    inline KisFixedPaintDeviceSP fetchFromCache(KisDabCacheUtils::DabRenderingResources *resources, const KisPaintInformation& info,
                                                QRect *dstDabRect);

    inline KisFixedPaintDeviceSP fetchDabCommon(const KoColorSpace *cs,
            KisColorSource *colorSource,
            const KoColor& color,
            const QPointF &cursorPoint,
            KisDabShape,
            const KisPaintInformation& info,
            qreal softnessFactor,
            QRect *dstDabRect,
            qreal lightnessStrength = 1.0,
            bool forceImageStamp = false);

private:

    struct Private;
    Private * const m_d;

};

#endif /* __KIS_DAB_CACHE_H */
