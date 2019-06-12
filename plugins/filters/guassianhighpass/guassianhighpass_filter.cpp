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
#include "guassianhighpass_filter.h"
#include <QBitArray>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColor.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <KoCompositeOps.h>

#include <kis_mask_generator.h>
#include <kis_gaussian_kernel.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoMixColorsOp.h>
#include <kis_paint_device.h>
#include "kis_lod_transform.h"

#include "wdg_guassianhighpass.h"
#include "ui_wdgguassianhighpass.h"
#include "KoColorSpaceTraits.h"
#include <KisSequentialIteratorProgress.h>


KisGuassianHighPassFilter::KisGuassianHighPassFilter() : KisFilter(id(), FiltersCategoryEdgeDetectionId, i18n("&Guassian High Pass..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsThreading(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisGuassianHighPassFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool /* useForMasks */) const
{
    return new KisWdgGuassianHighPass(parent);
}

KisFilterConfigurationSP KisGuassianHighPassFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("blurAmount", 1);
    return config;
}

void KisGuassianHighPassFilter::processImpl(KisPaintDeviceSP device,
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

    KisPaintDeviceSP blur = m_cachedPaintDevice.getDevice(device);
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

    m_cachedPaintDevice.putDevice(blur);
}


QRect KisGuassianHighPassFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfSize = config->getProperty("blurAmount", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfSize * 2, -halfSize * 2, halfSize * 2, halfSize * 2);
}

QRect KisGuassianHighPassFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfSize = config->getProperty("blurAmount", value) ? KisGaussianKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfSize, -halfSize, halfSize, halfSize);
}
