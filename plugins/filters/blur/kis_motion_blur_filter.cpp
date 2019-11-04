/*
 * This file is part of Krita
 *
 * Copyright (c) 2010 Edward Apap <schumifer@hotmail.com>
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


#include "kis_motion_blur_filter.h"
#include "kis_wdg_motion_blur.h"

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include "ui_wdg_motion_blur.h"

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_lod_transform.h"


#include <QPainter>

#include <math.h>


KisMotionBlurFilter::KisMotionBlurFilter() : KisFilter(id(), FiltersCategoryBlurId, i18n("&Motion Blur..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisMotionBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    return new KisWdgMotionBlur(parent);
}

KisFilterConfigurationSP KisMotionBlurFilter::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("blurAngle", 0);
    config->setProperty("blurLength", 5);

    return config;
}

void KisMotionBlurFilter::processImpl(KisPaintDeviceSP device,
                                      const QRect& rect,
                                      const KisFilterConfigurationSP _config,
                                      KoUpdater* progressUpdater
                                      ) const
{
    QPoint srcTopLeft = rect.topLeft();

    Q_ASSERT(device);

    KisFilterConfigurationSP config = _config ? _config : new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    uint blurAngle = 0;
    if (config->getProperty("blurAngle", value)) {
        blurAngle = value.toUInt();
    }

    KisLodTransformScalar t(device);

    uint blurLength = 0;
    if (config->getProperty("blurLength", value)) {
        blurLength = t.scale(value.toUInt());
    }

    if (blurLength == 0) {
        return;
    }

    QBitArray channelFlags;

    if (config) {
        channelFlags = config->channelFlags();
    }

    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }

    // convert angle to radians
    qreal angleRadians = blurAngle / 360.0 * 2 * M_PI;

    // construct image
    qreal halfWidth = blurLength / 2.0 * cos(angleRadians);
    qreal halfHeight = blurLength / 2.0 * sin(angleRadians);

    int kernelWidth = ceil(fabs(halfWidth)) * 2;
    int kernelHeight = ceil(fabs(halfHeight)) * 2;

    // check for zero dimensions (vertical/horizontal motion vectors)
    kernelWidth = (kernelWidth == 0) ? 1 : kernelWidth;
    kernelHeight = (kernelHeight == 0) ? 1 : kernelHeight;

    QImage kernelRepresentation(kernelWidth, kernelHeight, QImage::Format_RGB32);
    kernelRepresentation.fill(0);

    QPainter imagePainter(&kernelRepresentation);
    imagePainter.setRenderHint(QPainter::Antialiasing);
    imagePainter.setPen(QPen(QColor::fromRgb(255, 255, 255), 1.0));
    imagePainter.drawLine(QPointF(kernelWidth / 2 - halfWidth, kernelHeight / 2 + halfHeight),
                          QPointF(kernelWidth / 2 + halfWidth, kernelHeight / 2 - halfHeight));

    // construct kernel from image
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> motionBlurKernel(kernelHeight, kernelWidth);
    for (int j = 0; j < kernelHeight; ++j) {
        for (int i = 0; i < kernelWidth; ++i) {
            motionBlurKernel(j, i) = qRed(kernelRepresentation.pixel(i, j));
        }
    }

    // apply convolution
    KisConvolutionPainter painter(device);
    painter.setChannelFlags(channelFlags);
    painter.setProgress(progressUpdater);

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMatrix(motionBlurKernel, 0, motionBlurKernel.sum());
    painter.applyMatrix(kernel, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);
}

QRect KisMotionBlurFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    uint blurAngle = _config->getProperty("blurAngle", value) ? value.toUInt() : 0;
    uint blurLength = t.scale(_config->getProperty("blurLength", value) ? value.toUInt() : 5);

    qreal angleRadians = blurAngle / 360.0 * 2 * M_PI;
    const int halfWidth = ceil(fabs(blurLength / 2.0 * cos(angleRadians)));
    const int halfHeight = ceil(fabs(blurLength / 2.0 * cos(angleRadians)));

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}

QRect KisMotionBlurFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    uint blurAngle = _config->getProperty("blurAngle", value) ? value.toUInt() : 0;
    uint blurLength = t.scale(_config->getProperty("blurLength", value) ? value.toUInt() : 5);

    qreal angleRadians = blurAngle / 360.0 * 2 * M_PI;
    const int halfWidth = ceil(fabs(blurLength * cos(angleRadians)));
    const int halfHeight = ceil(fabs(blurLength * cos(angleRadians)));

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}
