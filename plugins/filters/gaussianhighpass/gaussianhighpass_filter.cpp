/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "gaussianhighpass_filter.h"
#include <QBitArray>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColor.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

#include <kis_mask_generator.h>
#include <kis_gaussian_kernel.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoMixColorsOp.h>
#include "kis_lod_transform.h"
#include <KoCompositeOpRegistry.h>

#include "wdg_gaussianhighpass.h"
#include "ui_wdggaussianhighpass.h"
#include "KoColorSpaceTraits.h"
#include <KisSequentialIteratorProgress.h>


KisGaussianHighPassFilter::KisGaussianHighPassFilter() : KisFilter(id(), FiltersCategoryEdgeDetectionId, i18n("&Gaussian High Pass..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsThreading(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisGaussianHighPassFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool /* useForMasks */) const
{
    return new KisWdgGaussianHighPass(parent);
}

KisFilterConfigurationSP KisGaussianHighPassFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("blurAmount", 1);
    return config;
}

void KisGaussianHighPassFilter::processImpl(KisPaintDeviceSP device,
                                   const QRect& applyRect,
                                   const KisFilterConfigurationSP config,
                                   KoUpdater *
                                   ) const
{
    QPointer<KoUpdater> convolutionUpdater = 0;
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    QVariant value;
    KisLodTransformScalar t(device);
    const qreal blurAmount = t.scale(config->getProperty("blurAmount", value) ? value.toDouble() : 1.0);
    QBitArray channelFlags = config->channelFlags();

    const QRect gaussNeedRect = this->neededRect(applyRect, config, device->defaultBounds()->currentLevelOfDetail());

    KisCachedPaintDevice::Guard d1(device, m_cachedPaintDevice);
    KisPaintDeviceSP blur = d1.device();
    KisPainter::copyAreaOptimizedOldData(gaussNeedRect.topLeft(), device, blur, gaussNeedRect);
    KisGaussianKernel::applyGaussian(blur, applyRect,
                                     blurAmount, blurAmount,
                                     channelFlags,
                                     convolutionUpdater,
                                     true); // make sure we craate an internal transaction on temp device
    
    KisPainter painter(device);
    painter.setCompositeOp(blur->colorSpace()->compositeOp(COMPOSITE_GRAIN_EXTRACT));
    painter.bitBlt(applyRect.topLeft(), blur, applyRect);
    painter.end();
}


QRect KisGaussianHighPassFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfSize = config->getProperty("blurAmount", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfSize * 2, -halfSize * 2, halfSize * 2, halfSize * 2);
}

QRect KisGaussianHighPassFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfSize = config->getProperty("blurAmount", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfSize, -halfSize, halfSize, halfSize);
}
