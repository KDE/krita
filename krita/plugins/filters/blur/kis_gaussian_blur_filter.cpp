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

#include <kcombobox.h>
#include <knuminput.h>

#include <KoCompositeOp.h>

#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>
#include <kis_iterators_pixel.h>

#include "ui_wdg_gaussian_blur.h"

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include <math.h>


KisGaussianBlurFilter::KisGaussianBlurFilter() : KisFilter(id(), categoryBlur(), i18n("&Gaussian Blur..."))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KisConfigWidget * KisGaussianBlurFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, const KisImageWSP image) const
{
    Q_UNUSED(image)
    return new KisWdgGaussianBlur(parent);
}

KisFilterConfiguration* KisGaussianBlurFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("horizRadius", 5);
    config->setProperty("vertRadius", 5);
    config->setProperty("lockAspect", true);

    return config;
}

void KisGaussianBlurFilter::process(KisConstProcessingInformation srcInfo,
                            KisProcessingInformation dstInfo,
                            const QSize& size,
                            const KisFilterConfiguration* config,
                            KoUpdater* progressUpdater
                           ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();

    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    if (!config) config = new KisFilterConfiguration(id().id(), 1);

    QVariant value;
    config->getProperty("horizRadius", value);
    uint horizontalRadius = value.toUInt();
    config->getProperty("vertRadius", value);
    uint verticalRadius = value.toUInt();

    // compute horizontal kernel
    uint horizKernelSize = horizontalRadius * 2 + 1;
    Matrix<qreal, Dynamic, Dynamic> horizGaussian(1, horizKernelSize);

    qreal horizSigma = horizontalRadius;
    const qreal horizMultiplicand = 1 / (2 * M_PI * horizSigma * horizSigma);
    const qreal horizExponentMultiplicand = 1 / (2 * horizSigma * horizSigma);

    for (uint x = 0; x < horizKernelSize; x++)
    {
        uint xDistance = qAbs((int)horizontalRadius - (int)x);
        horizGaussian(0, x) = horizMultiplicand * exp( -(qreal)((xDistance * xDistance) + (horizontalRadius * horizontalRadius)) * horizExponentMultiplicand );
    }

    // compute vertical kernel
    uint verticalKernelSize = verticalRadius * 2 + 1;
    Matrix<qreal, Dynamic, Dynamic> verticalGaussian(verticalKernelSize, 1);

    qreal verticalSigma = verticalRadius;
    const qreal verticalMultiplicand = 1 / (2 * M_PI * verticalSigma * verticalSigma);
    const qreal verticalExponentMultiplicand = 1 / (2 * verticalSigma * verticalSigma);

    for (uint y = 0; y < verticalKernelSize; y++)
    {
        uint yDistance = qAbs((int)verticalRadius - (int)y);
        verticalGaussian(y, 0) = verticalMultiplicand * exp( -(qreal)((yDistance * yDistance) + (verticalRadius * verticalRadius)) * verticalExponentMultiplicand );
    }

    if ( (horizontalRadius > 0) && (verticalRadius > 0) )
    {
        KisPaintDeviceSP interm = new KisPaintDevice(src->colorSpace());

        KisConvolutionKernelSP kernelHoriz = KisConvolutionKernel::fromMatrix(horizGaussian, 0, horizGaussian.sum());
        KisConvolutionKernelSP kernelVertical = KisConvolutionKernel::fromMatrix(verticalGaussian, 0, verticalGaussian.sum());

        KisConvolutionPainter horizPainter(interm, dstInfo.selection());
        horizPainter.setProgress(progressUpdater);
        horizPainter.applyMatrix(kernelHoriz, src, srcTopLeft, srcTopLeft, size, BORDER_REPEAT);

        KisConvolutionPainter verticalPainter(dst, dstInfo.selection());
        verticalPainter.setProgress(progressUpdater);
        verticalPainter.applyMatrix(kernelVertical, interm, srcTopLeft, dstTopLeft, size, BORDER_REPEAT);
    }
    else
    {
        if (horizontalRadius > 0)
        {
            KisConvolutionPainter painter(dst, dstInfo.selection());
            painter.setProgress(progressUpdater);

            KisConvolutionKernelSP kernelHoriz = KisConvolutionKernel::fromMatrix(horizGaussian, 0, horizGaussian.sum());
            painter.applyMatrix(kernelHoriz, src, srcTopLeft, dstTopLeft, size, BORDER_REPEAT);
        }

        if (verticalRadius > 0)
        {
            KisConvolutionPainter painter(dst, dstInfo.selection());
            painter.setProgress(progressUpdater);

            KisConvolutionKernelSP kernelVertical = KisConvolutionKernel::fromMatrix(verticalGaussian, 0, verticalGaussian.sum());
            painter.applyMatrix(kernelVertical, src, srcTopLeft, dstTopLeft, size, BORDER_REPEAT);
        }
    }
}