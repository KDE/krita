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

#include <kcombobox.h>
#include <knuminput.h>

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include "ui_wdg_lens_blur.h"

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include <QPainter>

#include <math.h>


KisLensBlurFilter::KisLensBlurFilter() : KisFilter(id(), categoryBlur(), i18n("&Lens Blur..."))
{
    setSupportsPainting(true);
    setSupportsIncrementalPainting(true);
    setSupportsAdjustmentLayers(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

KisConfigWidget * KisLensBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP image) const
{
    Q_UNUSED(image)
    return new KisWdgLensBlur(parent);
}

KisFilterConfiguration* KisLensBlurFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("irisShape", "Pentagon (5)");
    config->setProperty("irisRadius", 5);
    config->setProperty("irisRotation", 0);

    return config;
}

void KisLensBlurFilter::process(KisPaintDeviceSP device,
                            const QRect& rect,
                            const KisFilterConfiguration* config,
                            KoUpdater* progressUpdater
                           ) const
{
    QPoint srcTopLeft = rect.topLeft();

    Q_ASSERT(device != 0);

    if (!config) config = new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    config->getProperty("irisShape", value);
    QString irisShape = value.toString();
    config->getProperty("irisRadius", value);
    uint irisRadius = value.toUInt();
    config->getProperty("irisRotation", value);
    uint irisRotation = value.toUInt();

    if (irisRadius < 1)
        return;

    QBitArray channelFlags;
    if (config) {
        channelFlags = config->channelFlags();
    } 
    if (channelFlags.isEmpty() || !config) {
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }
    
    QPolygonF irisShapePoly;

    int sides = 1;
    qreal angle = 0;

    if (irisShape == "Triangle") sides = 3;
    else if (irisShape == "Quadrilateral (4)") sides = 4;
    else if (irisShape == "Pentagon (5)") sides = 5;
    else if (irisShape == "Hexagon (6)") sides = 6;
    else if (irisShape == "Heptagon (7)") sides = 7;
    else if (irisShape == "Octagon (8)") sides = 8;
    else return;

    for (int i = 0; i < sides; ++i) {
        irisShapePoly << QPointF(0.5 * cos(angle), 0.5 * sin(angle));
        angle += 2 * M_PI / sides;
    }

    QTransform transform;
    transform.rotate(irisRotation);
    transform.scale(irisRadius * 2, irisRadius * 2);

    QPolygonF transformedIris;
    for (int i = 0; i < irisShapePoly.count(); ++i) {
        transformedIris << irisShapePoly[i] * transform;
    }

    // find extremes to determine kernel size required
    qreal minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (int i = 0; i < transformedIris.count(); ++i) {
        if (transformedIris[i].x() < minX) minX = transformedIris[i].x();
        if (transformedIris[i].x() > maxX) maxX = transformedIris[i].x();
        if (transformedIris[i].y() < minY) minY = transformedIris[i].y();
        if (transformedIris[i].y() > maxY) maxY = transformedIris[i].y();
    }

    int kernelWidth = ceil(maxX) - ceil(minX);
    int kernelHeight = ceil(maxY) - ceil(minY);

    QImage kernelRepresentation(kernelWidth, kernelHeight, QImage::Format_RGB32);
    kernelRepresentation.fill(0);

    QPainter imagePainter(&kernelRepresentation);
    imagePainter.setRenderHint(QPainter::Antialiasing);
    imagePainter.setBrush(QColor::fromRgb(255, 255, 255));

    QTransform offsetTransform;
    offsetTransform.translate(-minX, -minY);
    imagePainter.setTransform(offsetTransform);
    imagePainter.drawPolygon(transformedIris, Qt::WindingFill);

    // construct kernel from image
    Matrix<qreal, Dynamic, Dynamic> irisKernel(kernelHeight, kernelWidth);
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
