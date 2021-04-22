/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDabCacheUtils.h"

#include "kis_brush.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_color_source.h"

#include <kis_pressure_sharpness_option.h>
#include <kis_texture_option.h>

#include <kundo2command.h>

namespace KisDabCacheUtils
{

DabRenderingResources::DabRenderingResources()
{
}

DabRenderingResources::~DabRenderingResources()
{
}

void DabRenderingResources::syncResourcesToSeqNo(int seqNo, const KisPaintInformation &info)
{
    brush->prepareForSeqNo(info, seqNo);
}

QRect correctDabRectWhenFetchedFromCache(const QRect &dabRect,
                                         const QSize &realDabSize)
{
    int diffX = (realDabSize.width() - dabRect.width()) / 2;
    int diffY = (realDabSize.height() - dabRect.height()) / 2;

    return QRect(dabRect.x() - diffX, dabRect.y() - diffY,
                 realDabSize.width() , realDabSize.height());
}

void generateDab(const DabGenerationInfo &di, DabRenderingResources *resources, KisFixedPaintDeviceSP *dab, bool forceNormalizedRGBAImageStamp)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(*dab);
    const KoColorSpace *cs = (*dab)->colorSpace();


    if (forceNormalizedRGBAImageStamp || resources->brush->brushApplication() == IMAGESTAMP) {
        *dab = resources->brush->paintDevice(cs, di.shape, di.info,
                                             di.subPixel.x(),
                                             di.subPixel.y(),
                                             forceNormalizedRGBAImageStamp);

    } else if (di.solidColorFill) {
        resources->brush->mask(*dab,
                               di.paintColor,
                               di.shape,
                               di.info,
                               di.subPixel.x(), di.subPixel.y(),
                               di.softnessFactor,
                               di.lightnessStrength);
    }
    else {
        if (!resources->colorSourceDevice ||
            *cs != *resources->colorSourceDevice->colorSpace()) {

            resources->colorSourceDevice = new KisPaintDevice(cs);
        }
        else {
            resources->colorSourceDevice->clear();
        }

        QRect maskRect(QPoint(), di.dstDabRect.size());
        resources->colorSource->colorize(resources->colorSourceDevice, maskRect, di.info.pos().toPoint());
        resources->colorSourceDevice->convertTo(cs);

        resources->brush->mask(*dab, resources->colorSourceDevice,
                               di.shape,
                               di.info,
                               di.subPixel.x(), di.subPixel.y(),
                               di.softnessFactor,
                               di.lightnessStrength);
    }

    if (!di.mirrorProperties.isEmpty()) {
        (*dab)->mirror(di.mirrorProperties.horizontalMirror,
                       di.mirrorProperties.verticalMirror);
    }
}

void postProcessDab(KisFixedPaintDeviceSP dab,
                    const QPoint &dabTopLeft,
                    const KisPaintInformation& info,
                    DabRenderingResources *resources)
{
    if (resources->sharpnessOption) {
        resources->sharpnessOption->applyThreshold(dab, info);
    }

    if (resources->textureOption) {
        resources->textureOption->apply(dab, dabTopLeft, info);
    }
}

}

