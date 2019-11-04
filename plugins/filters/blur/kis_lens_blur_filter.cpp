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


#include "kis_lens_blur_filter.h"
#include "kis_wdg_lens_blur.h"

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include "ui_wdg_lens_blur.h"

#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_lod_transform.h"


#include <QPainter>

#include <math.h>


KisLensBlurFilter::KisLensBlurFilter() : KisFilter(id(), FiltersCategoryBlurId, i18n("&Lens Blur..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisLensBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    return new KisWdgLensBlur(parent);
}

QSize KisLensBlurFilter::getKernelHalfSize(const KisFilterConfigurationSP config, int lod)
{
    QPolygonF iris = getIrisPolygon(config, lod);
    QRect rect = iris.boundingRect().toAlignedRect();

    int w = std::ceil(qreal(rect.width()) / 2.0);
    int h = std::ceil(qreal(rect.height()) / 2.0);

    return QSize(w, h);
}

KisFilterConfigurationSP KisLensBlurFilter::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("irisShape", "Pentagon (5)");
    config->setProperty("irisRadius", 5);
    config->setProperty("irisRotation", 0);

    QSize halfSize = getKernelHalfSize(config, 0);
    config->setProperty("halfWidth", halfSize.width());
    config->setProperty("halfHeight", halfSize.height());

    return config;
}

QPolygonF KisLensBlurFilter::getIrisPolygon(const KisFilterConfigurationSP config, int lod)
{
    KIS_ASSERT_RECOVER(config) { return QPolygonF(); }

    KisLodTransformScalar t(lod);

    QVariant value;
    config->getProperty("irisShape", value);
    QString irisShape = value.toString();
    config->getProperty("irisRadius", value);
    uint irisRadius = t.scale(value.toUInt());
    config->getProperty("irisRotation", value);
    uint irisRotation = value.toUInt();

    if (irisRadius < 1)
        return QPolygon();

    QPolygonF irisShapePoly;

    int sides = 1;
    qreal angle = 0;

    if (irisShape == "Triangle") sides = 3;
    else if (irisShape == "Quadrilateral (4)") sides = 4;
    else if (irisShape == "Pentagon (5)") sides = 5;
    else if (irisShape == "Hexagon (6)") sides = 6;
    else if (irisShape == "Heptagon (7)") sides = 7;
    else if (irisShape == "Octagon (8)") sides = 8;
    else return QPolygonF();

    for (int i = 0; i < sides; ++i) {
        irisShapePoly << QPointF(0.5 * cos(angle), 0.5 * sin(angle));
        angle += 2 * M_PI / sides;
    }

    QTransform transform;
    transform.rotate(irisRotation);
    transform.scale(irisRadius * 2, irisRadius * 2);

    QPolygonF transformedIris = transform.map(irisShapePoly);

    return transformedIris;
}

void KisLensBlurFilter::processImpl(KisPaintDeviceSP device,
                                    const QRect& rect,
                                    const KisFilterConfigurationSP _config,
                                    KoUpdater* progressUpdater
                                    ) const
{
    QPoint srcTopLeft = rect.topLeft();

    Q_ASSERT(device != 0);

    KisFilterConfigurationSP config = _config ? _config : new KisFilterConfiguration(id().id(), 1);

    QBitArray channelFlags;
    if (config) {
        channelFlags = config->channelFlags();
    }
    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }

    const int lod = device->defaultBounds()->currentLevelOfDetail();
    QPolygonF transformedIris = getIrisPolygon(config, lod);
    if (transformedIris.isEmpty()) return;

    QRectF boundingRect = transformedIris.boundingRect();

    int kernelWidth = boundingRect.toAlignedRect().width();
    int kernelHeight = boundingRect.toAlignedRect().height();

    QImage kernelRepresentation(kernelWidth, kernelHeight, QImage::Format_RGB32);
    kernelRepresentation.fill(0);

    QPainter imagePainter(&kernelRepresentation);
    imagePainter.setRenderHint(QPainter::Antialiasing);
    imagePainter.setBrush(QColor::fromRgb(255, 255, 255));

    QTransform offsetTransform;
    offsetTransform.translate(-boundingRect.x(), -boundingRect.y());
    imagePainter.setTransform(offsetTransform);
    imagePainter.drawPolygon(transformedIris, Qt::WindingFill);

    // construct kernel from image
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> irisKernel(kernelHeight, kernelWidth);
    for (int j = 0; j < kernelHeight; ++j) {
        for (int i = 0; i < kernelWidth; ++i) {
            irisKernel(j, i) = qRed(kernelRepresentation.pixel(i, j));
        }
    }

    // apply convolution
    KisConvolutionPainter painter(device);
    painter.setChannelFlags(channelFlags);
    painter.setProgress(progressUpdater);

    KisConvolutionKernelSP kernel = KisConvolutionKernel::fromMatrix(irisKernel, 0, irisKernel.sum());
    painter.applyMatrix(kernel, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);
}

QRect KisLensBlurFilter::neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    const int halfWidth = t.scale(_config->getProperty("halfWidth", value) ? value.toUInt() : 5);
    const int halfHeight = t.scale(_config->getProperty("halfHeight", value) ? value.toUInt() : 5);

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}

QRect KisLensBlurFilter::changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    const int halfWidth = t.scale(_config->getProperty("halfWidth", value) ? value.toUInt() : 5);
    const int halfHeight = t.scale(_config->getProperty("halfHeight", value) ? value.toUInt() : 5);

    return rect.adjusted(-halfWidth, -halfHeight, halfWidth, halfHeight);
}
