/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_gaussian_kernel.h"

#include "kis_convolution_kernel.h"
#include <kis_convolution_painter.h>
#include <QRect>


qreal KisGaussianKernel::sigmaFromRadius(qreal radius)
{
    return 0.3 * radius + 0.3;
}

int KisGaussianKernel::kernelSizeFromRadius(qreal radius)
{
    return 6 * ceil(sigmaFromRadius(radius)) + 1;
}


Matrix<qreal, Dynamic, Dynamic>
KisGaussianKernel::createHorizontalMatrix(qreal radius)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Matrix<qreal, Dynamic, Dynamic> matrix(1, kernelSize);

    const qreal sigma = sigmaFromRadius(radius);
    const qreal multiplicand = 1 / (sqrt(2 * M_PI * sigma * sigma));
    const qreal exponentMultiplicand = 1 / (2 * sigma * sigma);

    /**
     * The kernel size should always be odd, then the position of the
     * central pixel can be easily calculated
     */
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    for (int x = 0; x < kernelSize; x++) {
        qreal xDistance = center - x;
        matrix(0, x) = multiplicand * exp( -xDistance * xDistance * exponentMultiplicand );
    }

    return matrix;
}

Matrix<qreal, Dynamic, Dynamic>
KisGaussianKernel::createVerticalMatrix(qreal radius)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Matrix<qreal, Dynamic, Dynamic> matrix(kernelSize, 1);

    const qreal sigma = sigmaFromRadius(radius);
    const qreal multiplicand = 1 / (sqrt(2 * M_PI * sigma * sigma));
    const qreal exponentMultiplicand = 1 / (2 * sigma * sigma);

    /**
     * The kernel size should always be odd, then the position of the
     * central pixel can be easily calculated
     */
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    for (int y = 0; y < kernelSize; y++) {
        qreal yDistance = center - y;
        matrix(y, 0) = multiplicand * exp( -yDistance * yDistance * exponentMultiplicand );
    }

    return matrix;
}

KisConvolutionKernelSP
KisGaussianKernel::createHorizontalKernel(qreal radius)
{
    Matrix<qreal, Dynamic, Dynamic> matrix = createHorizontalMatrix(radius);
    return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
}

KisConvolutionKernelSP
KisGaussianKernel::createVerticalKernel(qreal radius)
{
    Matrix<qreal, Dynamic, Dynamic> matrix = createVerticalMatrix(radius);
    return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
}

void KisGaussianKernel::applyGaussian(KisPaintDeviceSP device,
                                      const QRect& rect,
                                      qreal xRadius, qreal yRadius,
                                      const QBitArray &channelFlags,
                                      KoUpdater *progressUpdater)
{
    QPoint srcTopLeft = rect.topLeft();

    if (xRadius > 0.0 && yRadius > 0.0) {
        KisPaintDeviceSP interm = new KisPaintDevice(device->colorSpace());

        KisConvolutionKernelSP kernelHoriz = KisGaussianKernel::createHorizontalKernel(xRadius);
        KisConvolutionKernelSP kernelVertical = KisGaussianKernel::createVerticalKernel(yRadius);

        qreal verticalCenter = qreal(kernelVertical->height()) / 2.0;

        KisConvolutionPainter horizPainter(interm);
        horizPainter.setChannelFlags(channelFlags);
        horizPainter.setProgress(progressUpdater);
        horizPainter.applyMatrix(kernelHoriz, device,
                                 srcTopLeft - QPoint(0, ceil(verticalCenter)),
                                 srcTopLeft - QPoint(0, ceil(verticalCenter)),
                                 rect.size() + QSize(0, 2 * ceil(verticalCenter)), BORDER_REPEAT);


        KisConvolutionPainter verticalPainter(device);
        verticalPainter.setChannelFlags(channelFlags);
        verticalPainter.setProgress(progressUpdater);
        verticalPainter.applyMatrix(kernelVertical, interm, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);

    } else if (xRadius > 0.0) {
        KisConvolutionPainter painter(device);
        painter.setChannelFlags(channelFlags);
        painter.setProgress(progressUpdater);

        KisConvolutionKernelSP kernelHoriz = KisGaussianKernel::createHorizontalKernel(xRadius);
        painter.applyMatrix(kernelHoriz, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);

    } else if (yRadius > 0.0) {
        KisConvolutionPainter painter(device);
        painter.setChannelFlags(channelFlags);
        painter.setProgress(progressUpdater);

        KisConvolutionKernelSP kernelVertical = KisGaussianKernel::createVerticalKernel(yRadius);
        painter.applyMatrix(kernelVertical, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);
    }
}
