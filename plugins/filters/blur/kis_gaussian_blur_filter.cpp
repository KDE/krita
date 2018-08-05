/*
 * This file is part of Krita
 *
 * Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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


#include "kis_gaussian_blur_filter.h"
#include "kis_wdg_gaussian_blur.h"

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_gaussian_kernel.h>

#include "ui_wdg_gaussian_blur.h"

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_lod_transform.h"


#include <math.h>


KisGaussianBlurFilter::KisGaussianBlurFilter() : KisFilter(id(), categoryBlur(), i18n("&Gaussian Blur..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisGaussianBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP) const
{
    return new KisWdgGaussianBlur(parent);
}

KisFilterConfigurationSP KisGaussianBlurFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("horizRadius", 5);
    config->setProperty("vertRadius", 5);
    config->setProperty("lockAspect", true);

    return config;
}

void KisGaussianBlurFilter::processImpl(KisPaintDeviceSP device,
                                        const QRect& rect,
                                        const KisFilterConfigurationSP _config,
                                        KoUpdater* progressUpdater
                                        ) const
{
    Q_ASSERT(device != 0);

    KisFilterConfigurationSP config = _config ? _config : new KisFilterConfiguration(id().id(), 1);

    KisLodTransformScalar t(device);

    QVariant value;
    config->getProperty("horizRadius", value);
    float horizontalRadius = t.scale(value.toFloat());
    config->getProperty("vertRadius", value);
    float verticalRadius = t.scale(value.toFloat());

    QBitArray channelFlags;
    if (config) {
        channelFlags = config->channelFlags();
    }
    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }

    KisGaussianKernel::applyGaussian(device, rect,
                                     horizontalRadius, verticalRadius,
                                     channelFlags, progressUpdater);
}

QRect KisGaussianBlurFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    /**
     * NOTE: integer division by two is done on purpose,
     *       because the kernel size is always odd
     */
    const int halfWidth = _config->getProperty("horizRadius", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;
    const int halfHeight = _config->getProperty("vertRadius", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}

QRect KisGaussianBlurFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfWidth = _config->getProperty("horizRadius", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;
    const int halfHeight = _config->getProperty("vertRadius", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfWidth, -halfHeight, halfWidth, halfHeight);
}
