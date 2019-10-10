/*
 * This file is part of Krita
 *
 * Copyright (c) 2019 Miguel Lopez <reptillia39@live.com>
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

KisFilterConfigurationSP KisGaussianHighPassFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("blurAmount", 1);
    return config;
}

void KisGaussianHighPassFilter::processImpl(KisPaintDeviceSP device,
                                   const QRect& applyRect,
                                   const KisFilterConfigurationSP _config,
                                   KoUpdater *
                                   ) const
{
    QPointer<KoUpdater> convolutionUpdater = 0;

    KisFilterConfigurationSP config = _config ? _config : new KisFilterConfiguration(id().id(), 1);
    
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
                                     true); // make sure we cerate an internal transaction on temp device
    
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
