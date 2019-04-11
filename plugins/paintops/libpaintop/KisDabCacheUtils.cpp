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


void generateDab(const DabGenerationInfo &di, DabRenderingResources *resources, KisFixedPaintDeviceSP *dab)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(*dab);
    const KoColorSpace *cs = (*dab)->colorSpace();


    if (resources->brush->brushType() == IMAGE || resources->brush->brushType() == PIPE_IMAGE) {
        *dab = resources->brush->paintDevice(cs, di.shape, di.info,
                                            di.subPixel.x(),
                                            di.subPixel.y());
    } else if (di.solidColorFill) {
        resources->brush->mask(*dab,
                               di.paintColor,
                               di.shape,
                               di.info,
                               di.subPixel.x(), di.subPixel.y(),
                               di.softnessFactor);
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
                               di.softnessFactor);
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

