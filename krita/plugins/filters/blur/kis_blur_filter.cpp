/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_blur_filter.h"
#include <kcombobox.h>
#include <knuminput.h>

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_iterators_pixel.h>

#include "kis_wdg_blur.h"
#include "ui_wdgblur.h"
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_mask_generator.h"

KisBlurFilter::KisBlurFilter() : KisFilter(id(), categoryBlur(), i18n("&Blur..."))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(true);
    setSupportsAdjustmentLayers(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP image) const
{
    Q_UNUSED(image)
    return new KisWdgBlur(parent);
}

KisFilterConfiguration* KisBlurFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("halfWidth", 5);
    config->setProperty("halfHeight", 5);
    config->setProperty("rotate", 0);
    config->setProperty("strength", 0);
    config->setProperty("shape", 0);
    return config;
}

void KisBlurFilter::process(KisConstProcessingInformation srcInfo,
                            KisProcessingInformation dstInfo,
                            const QSize& size,
                            const KisFilterConfiguration* config,
                            KoUpdater* progressUpdater
                           ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    if (!config) config = new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    int shape = (config->getProperty("shape", value)) ? value.toInt() : 0;
    uint halfWidth = (config->getProperty("halfWidth", value)) ? value.toUInt() : 5;
    uint width = 2 * halfWidth + 1;
    uint halfHeight = (config->getProperty("halfHeight", value)) ? value.toUInt() : 5;
    uint height = 2 * halfHeight + 1;
    int rotate = (config->getProperty("rotate", value)) ? value.toInt() : 0;
    int strength = 100 - (config->getProperty("strength", value)) ? value.toUInt() : 0;

    int hFade = (halfWidth * strength) / 100;
    int vFade = (halfHeight * strength) / 100;

    KisMaskGenerator* kas;
    dbgKrita << width << "" << height << "" << hFade << "" << vFade;
    switch (shape) {
    case 1:
        kas = new KisRectangleMaskGenerator(width, height , hFade, vFade);
        break;
    case 0:
    default:
        kas = new KisCircleMaskGenerator(width, height, hFade, vFade);
        break;
    }

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas, rotate * M_PI / 180.0);
    delete kas;
    KisConvolutionPainter painter(dst, dstInfo.selection());
    painter.setProgress(progressUpdater);
    painter.applyMatrix(kernel, src, srcTopLeft, dstTopLeft, size, BORDER_REPEAT);

}

QRect KisBlurFilter::neededRect(const QRect & rect, const KisFilterConfiguration* _config) const
{
    QVariant value;
    uint halfWidth = (_config->getProperty("halfWidth", value)) ? value.toUInt() : 5;
    uint halfHeight = (_config->getProperty("halfHeight", value)) ? value.toUInt() : 5;

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}

QRect KisBlurFilter::changedRect(const QRect & rect, const KisFilterConfiguration* _config) const
{
    QVariant value;
    uint halfWidth = (_config->getProperty("halfWidth", value)) ? value.toUInt() : 5;
    uint halfHeight = (_config->getProperty("halfHeight", value)) ? value.toUInt() : 5;

    return rect.adjusted(-halfWidth, -halfHeight, halfWidth, halfHeight);
}
