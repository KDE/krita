/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_blur_filter.h"

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include "kis_wdg_blur.h"
#include "ui_wdgblur.h"
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_mask_generator.h"
#include "kis_lod_transform.h"


KisBlurFilter::KisBlurFilter() : KisFilter(id(), FiltersCategoryBlurId, i18n("&Blur..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    return new KisWdgBlur(parent);
}

KisFilterConfigurationSP KisBlurFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("lockAspect", true);
    config->setProperty("halfWidth", 5);
    config->setProperty("halfHeight", 5);
    config->setProperty("rotate", 0);
    config->setProperty("strength", 0);
    config->setProperty("shape", 0);
    return config;
}

void KisBlurFilter::processImpl(KisPaintDeviceSP device,
                                const QRect& rect,
                                const KisFilterConfigurationSP config,
                                KoUpdater* progressUpdater
                                ) const
{
    QPoint srcTopLeft = rect.topLeft();
    Q_ASSERT(device != 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    KisLodTransformScalar t(device);

    QVariant value;
    const uint halfWidth = t.scale((config->getProperty("halfWidth", value)) ? value.toUInt() : 5);
    const uint halfHeight = t.scale((config->getProperty("halfHeight", value)) ? value.toUInt() : 5);

    int shape = (config->getProperty("shape", value)) ? value.toInt() : 0;
    uint width = 2 * halfWidth + 1;
    uint height = 2 * halfHeight + 1;
    qreal aspectRatio = (qreal) height / width;
    int rotate = (config->getProperty("rotate", value)) ? value.toInt() : 0;
    qreal strength = (config->getProperty("strength", value) ? value.toUInt() : 0) / (qreal) 100;
    qreal hFade = strength;
    qreal vFade = strength;

    KisMaskGenerator* kas;
    dbgKrita << width << "" << height << "" << hFade << "" << vFade;
    switch (shape) {
    case 1:
        kas = new KisRectangleMaskGenerator(width, aspectRatio, hFade, vFade, 2, true);
        break;
    case 0:
    default:
        kas = new KisCircleMaskGenerator(width, aspectRatio, hFade, vFade, 2, true);
        break;
    }

    QBitArray channelFlags;
    if (config) {
        channelFlags = config->channelFlags();
    }
    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMaskGenerator(kas, rotate * M_PI / 180.0);
    delete kas;
    KisConvolutionPainter painter(device);
    painter.setChannelFlags(channelFlags);
    painter.setProgress(progressUpdater);
    painter.applyMatrix(kernel, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);

}

QRect KisBlurFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    const int halfWidth = t.scale(_config->getProperty("halfWidth", value) ? value.toUInt() : 5);
    const int halfHeight = t.scale(_config->getProperty("halfHeight", value) ? value.toUInt() : 5);

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}

QRect KisBlurFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    const int halfWidth = t.scale(_config->getProperty("halfWidth", value) ? value.toUInt() : 5);
    const int halfHeight = t.scale(_config->getProperty("halfHeight", value) ? value.toUInt() : 5);

    return rect.adjusted(-halfWidth, -halfHeight, halfWidth, halfHeight);
}
