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

#include "kis_dab_cache_base.h"

#include <KoColor.h>
#include "kis_color_source.h"
#include "kis_paint_device.h"
#include "kis_brush.h"
#include <kis_pressure_mirror_option.h>
#include <kis_pressure_sharpness_option.h>
#include <kis_texture_option.h>
#include <kis_precision_option.h>
#include <kis_fixed_paint_device.h>
#include <brushengine/kis_paintop.h>

#include <kundo2command.h>

struct PrecisionValues {
    qreal angle;
    qreal sizeFrac;
    qreal subPixel;
    qreal softnessFactor;
};

const qreal eps = 1e-6;
static const PrecisionValues precisionLevels[] = {
    {M_PI / 180, 0.05,   1, 0.01},
    {M_PI / 180, 0.01,   1, 0.01},
    {M_PI / 180,    0,   1, 0.01},
    {M_PI / 180,    0, 0.5, 0.01},
    {eps,         0, eps,  eps}
};

struct KisDabCacheBase::SavedDabParameters {
    KoColor color;
    qreal angle;
    int width;
    int height;
    qreal subPixelX;
    qreal subPixelY;
    qreal softnessFactor;
    int index;
    MirrorProperties mirrorProperties;

    bool compare(const SavedDabParameters &rhs, int precisionLevel) const {
        const PrecisionValues &prec = precisionLevels[precisionLevel];

        return color == rhs.color &&
               qAbs(angle - rhs.angle) <= prec.angle &&
               qAbs(width - rhs.width) <= (int)(prec.sizeFrac * width) &&
               qAbs(height - rhs.height) <= (int)(prec.sizeFrac * height) &&
               qAbs(subPixelX - rhs.subPixelX) <= prec.subPixel &&
               qAbs(subPixelY - rhs.subPixelY) <= prec.subPixel &&
               qAbs(softnessFactor - rhs.softnessFactor) <= prec.softnessFactor &&
               index == rhs.index &&
               mirrorProperties.horizontalMirror == rhs.mirrorProperties.horizontalMirror &&
               mirrorProperties.verticalMirror == rhs.mirrorProperties.verticalMirror;
    }
};

struct KisDabCacheBase::Private {

    Private()
        : mirrorOption(0),
          precisionOption(0),
          subPixelPrecisionDisabled(false)
    {}

    KisPressureMirrorOption *mirrorOption;
    KisPrecisionOption *precisionOption;
    bool subPixelPrecisionDisabled;

    SavedDabParameters lastSavedDabParameters;

    static qreal positiveFraction(qreal x);
};



KisDabCacheBase::KisDabCacheBase()
    : m_d(new Private())
{
}

KisDabCacheBase::~KisDabCacheBase()
{
    delete m_d;
}

void KisDabCacheBase::setMirrorPostprocessing(KisPressureMirrorOption *option)
{
    m_d->mirrorOption = option;
}

void KisDabCacheBase::setPrecisionOption(KisPrecisionOption *option)
{
    m_d->precisionOption = option;
}

void KisDabCacheBase::disableSubpixelPrecision()
{
    m_d->subPixelPrecisionDisabled = true;
}

inline KisDabCacheBase::SavedDabParameters
KisDabCacheBase::getDabParameters(KisBrushSP brush,
                              const KoColor& color,
                              KisDabShape const& shape,
                              const KisPaintInformation& info,
                              double subPixelX, double subPixelY,
                              qreal softnessFactor,
                              MirrorProperties mirrorProperties)
{
    SavedDabParameters params;

    params.color = color;
    params.angle = shape.rotation();
    params.width = brush->maskWidth(shape, subPixelX, subPixelY, info);
    params.height = brush->maskHeight(shape, subPixelX, subPixelY, info);
    params.subPixelX = subPixelX;
    params.subPixelY = subPixelY;
    params.softnessFactor = softnessFactor;
    params.index = brush->brushIndex(info);
    params.mirrorProperties = mirrorProperties;

    return params;
}

bool KisDabCacheBase::needSeparateOriginal(KisTextureProperties *textureOption,
                                           KisPressureSharpnessOption *sharpnessOption) const
{
    return (textureOption && textureOption->m_enabled) ||
           (sharpnessOption && sharpnessOption->isChecked());
}

struct KisDabCacheBase::DabPosition {
    DabPosition(const QRect &_rect,
                const QPointF &_subPixel,
                qreal _realAngle)
        : rect(_rect),
          subPixel(_subPixel),
          realAngle(_realAngle) {
    }

    QRect rect;
    QPointF subPixel;
    qreal realAngle;
};

qreal KisDabCacheBase::Private::positiveFraction(qreal x) {
    qint32 unused = 0;
    qreal fraction = 0.0;
    KisPaintOp::splitCoordinate(x, &unused, &fraction);

    return fraction;
}

inline
KisDabCacheBase::DabPosition
KisDabCacheBase::calculateDabRect(KisBrushSP brush,
                                  const QPointF &cursorPoint,
                                  KisDabShape shape,
                                  const KisPaintInformation& info,
                                  const MirrorProperties &mirrorProperties,
                                  KisPressureSharpnessOption *sharpnessOption)
{
    qint32 x = 0, y = 0;
    qreal subPixelX = 0.0, subPixelY = 0.0;

    if (mirrorProperties.coordinateSystemFlipped) {
        shape = KisDabShape(shape.scale(), shape.ratio(), 2 * M_PI - shape.rotation());
    }

    QPointF hotSpot = brush->hotSpot(shape, info);
    QPointF pt = cursorPoint - hotSpot;

    if (sharpnessOption) {
        sharpnessOption->apply(info, pt, x, y, subPixelX, subPixelY);
    }
    else {
        KisPaintOp::splitCoordinate(pt.x(), &x, &subPixelX);
        KisPaintOp::splitCoordinate(pt.y(), &y, &subPixelY);
    }

    if (m_d->subPixelPrecisionDisabled) {
        subPixelX = 0;
        subPixelY = 0;
    }

    if (qIsNaN(subPixelX)) {
        subPixelX = 0;
    }

    if (qIsNaN(subPixelY)) {
        subPixelY = 0;
    }

    int width = brush->maskWidth(shape, subPixelX, subPixelY, info);
    int height = brush->maskHeight(shape, subPixelX, subPixelY, info);

    if (mirrorProperties.horizontalMirror) {
        subPixelX = Private::positiveFraction(-(cursorPoint.x() + hotSpot.x()));
        width = brush->maskWidth(shape, subPixelX, subPixelY, info);
        x = qRound(cursorPoint.x() + subPixelX + hotSpot.x()) - width;
    }

    if (mirrorProperties.verticalMirror) {
        subPixelY = Private::positiveFraction(-(cursorPoint.y() + hotSpot.y()));
        height = brush->maskHeight(shape, subPixelX, subPixelY, info);
        y = qRound(cursorPoint.y() + subPixelY + hotSpot.y()) - height;
    }

    return DabPosition(QRect(x, y, width, height),
                       QPointF(subPixelX, subPixelY),
                       shape.rotation());
}

void KisDabCacheBase::fetchDabGenerationInfo(bool hasDabInCache,
                                             KisDabCacheUtils::DabRenderingResources *resources,
                                             const KisDabCacheUtils::DabRequestInfo &request,
                                             KisDabCacheUtils::DabGenerationInfo *di,
                                             bool *shouldUseCache)
{
    di->info = request.info;
    di->softnessFactor = request.softnessFactor;

    if (m_d->mirrorOption) {
        di->mirrorProperties = m_d->mirrorOption->apply(request.info);
    }

    DabPosition position = calculateDabRect(resources->brush,
                                            request.cursorPoint,
                                            request.shape,
                                            request.info,
                                            di->mirrorProperties,
                                            resources->sharpnessOption.data());
    di->shape = KisDabShape(request.shape.scale(), request.shape.ratio(), position.realAngle);
    di->dstDabRect = position.rect;
    di->subPixel = position.subPixel;

    di->solidColorFill = !resources->colorSource || resources->colorSource->isUniformColor();
    di->paintColor = resources->colorSource && resources->colorSource->isUniformColor() ?
                resources->colorSource->uniformColor() : request.color;

    SavedDabParameters newParams = getDabParameters(resources->brush,
                                                    di->paintColor,
                                                    di->shape,
                                                    di->info,
                                                    di->subPixel.x(),
                                                    di->subPixel.y(),
                                                    di->softnessFactor,
                                                    di->mirrorProperties);

    int precisionLevel = 4;
    if (m_d->precisionOption) {
        const int effectiveDabSize = qMin(newParams.width, newParams.height);
        precisionLevel = m_d->precisionOption->effectivePrecisionLevel(effectiveDabSize) - 1;
    }

    *shouldUseCache = hasDabInCache && di->solidColorFill &&
            newParams.compare(m_d->lastSavedDabParameters, precisionLevel);

    if (!*shouldUseCache) {
        m_d->lastSavedDabParameters = newParams;
    }

    di->needsPostprocessing = needSeparateOriginal(resources->textureOption.data(), resources->sharpnessOption.data());
}

