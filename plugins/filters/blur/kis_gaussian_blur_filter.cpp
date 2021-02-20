/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "kis_gaussian_blur_filter.h"
#include "kis_wdg_gaussian_blur.h"

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_gaussian_kernel.h>

#include "ui_wdg_gaussian_blur.h"

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_lod_transform.h"


#include <math.h>


KisGaussianBlurFilter::KisGaussianBlurFilter() : KisFilter(id(), FiltersCategoryBlurId, i18n("&Gaussian Blur..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisGaussianBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool usedForMasks) const
{
    return new KisWdgGaussianBlur(usedForMasks, parent);
}

KisFilterConfigurationSP KisGaussianBlurFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("horizRadius", 5);
    config->setProperty("vertRadius", 5);
    config->setProperty("lockAspect", true);

    return config;
}

void KisGaussianBlurFilter::processImpl(KisPaintDeviceSP device,
                                        const QRect& rect,
                                        const KisFilterConfigurationSP config,
                                        KoUpdater* progressUpdater
                                        ) const
{
    Q_ASSERT(device != 0);

    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

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

bool KisGaussianBlurFilter::configurationAllowedForMask(KisFilterConfigurationSP config) const
{
    //ENTER_FUNCTION() << config->getFloat("horizRadius", 5.0) << config->getFloat("vertRadius", 5.0);

    const float maxRadiusForMask = 100.0;

    return config->getFloat("horizRadius", 5.0) <= maxRadiusForMask &&
            config->getFloat("vertRadius", 5.0) <= maxRadiusForMask;
}

void KisGaussianBlurFilter::fixLoadedFilterConfigurationForMasks(KisFilterConfigurationSP config) const
{
    ENTER_FUNCTION();

    const float maxRadiusForMask = 100.0;

    if (config->getFloat("horizRadius", 5.0) > maxRadiusForMask) {
        config->setProperty("horizRadius", maxRadiusForMask);
    }

    if (config->getFloat("vertRadius", 5.0) > maxRadiusForMask) {
        config->setProperty("vertRadius", maxRadiusForMask);
    }
}
