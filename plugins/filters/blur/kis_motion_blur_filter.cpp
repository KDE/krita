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

namespace {
struct MotionBlurProperties
{
    MotionBlurProperties(KisFilterConfigurationSP config, const KisLodTransformScalar &t)
    {
        const int blurAngle = config->getInt("blurAngle", 0);
        const int blurLength = config->getInt("blurLength", 5);

        // convert angle to radians
        const qreal angleRadians = kisDegreesToRadians(qreal(blurAngle));

        // construct image
        const qreal halfWidth = 0.5 * t.scale(blurLength) * cos(angleRadians);
        const qreal halfHeight = 0.5 * t.scale(blurLength) * sin(angleRadians);

        kernelHalfSize.rwidth() = ceil(fabs(halfWidth));
        kernelHalfSize.rheight() = ceil(fabs(halfHeight));
        kernelSize = kernelHalfSize * 2 + QSize(1, 1);
        this->blurLength = blurLength;


        QPointF p1(0.5 * kernelSize.width(), 0.5 * kernelSize.height());
        QPointF p2(halfWidth, halfHeight);
        motionLine = QLineF(p1 - p2, p1 + p2);
    }

    int blurLength;
    QSize kernelSize;
    QSize kernelHalfSize;
    QLineF motionLine;
};
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

    KisLodTransformScalar t(device);
    MotionBlurProperties props(config, t);

    if (props.blurLength == 0) {
        return;
    }

    QBitArray channelFlags;

    if (config) {
        channelFlags = config->channelFlags();
    }

    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }

    QImage kernelRepresentation(props.kernelSize, QImage::Format_RGB32);
    kernelRepresentation.fill(0);

    QPainter imagePainter(&kernelRepresentation);
    imagePainter.setRenderHint(QPainter::Antialiasing);
    imagePainter.setPen(QPen(QColor::fromRgb(255, 255, 255), 1.0));
    imagePainter.drawLine(props.motionLine);

    // construct kernel from image
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> motionBlurKernel(props.kernelSize.height(), props.kernelSize.width());
    for (int j = 0; j < props.kernelSize.height(); ++j) {
        for (int i = 0; i < props.kernelSize.width(); ++i) {
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
    MotionBlurProperties props(_config, t);
    return rect.adjusted(-props.kernelHalfSize.width(), -props.kernelHalfSize.height(), props.kernelHalfSize.width(), props.kernelHalfSize.height());
}

QRect KisMotionBlurFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    return neededRect(rect, _config, lod);
}
