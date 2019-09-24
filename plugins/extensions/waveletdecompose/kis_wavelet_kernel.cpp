/*
 *  Copyright (c) 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
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

#include "kis_wavelet_kernel.h"

#include "kis_convolution_kernel.h"
#include <kis_convolution_painter.h>
#include <QRect>



int KisWaveletKernel::kernelSizeFromRadius(qreal radius)
{
    return 2 * ceil(radius) + 1;
}

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
KisWaveletKernel::createHorizontalMatrix(qreal radius)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(1, kernelSize);

    /**
     * The kernel size should always be odd, then the position of the
     * central pixel can be easily calculated
     */
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    for (int x = 0; x < kernelSize; x++) {
        if (x == 0 || x == kernelSize - 1)
            matrix(0, x) = 0.25;
        else if (x == center)
            matrix(0, x) = 0.5;
        else
            matrix(0, x) = 0;
    }

    return matrix;
}

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
KisWaveletKernel::createVerticalMatrix(qreal radius)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, 1);


    /**
     * The kernel size should always be odd, then the position of the
     * central pixel can be easily calculated
     */
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    for (int y = 0; y < kernelSize; y++) {
        if (y == 0 || y == kernelSize - 1)
            matrix(y, 0) = 0.25;
        else if (y == center)
            matrix(y, 0) = 0.5;
        else
            matrix(y, 0) = 0;
    }

    return matrix;
}

KisConvolutionKernelSP
KisWaveletKernel::createHorizontalKernel(qreal radius)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createHorizontalMatrix(radius);
    return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
}

KisConvolutionKernelSP
KisWaveletKernel::createVerticalKernel(qreal radius)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createVerticalMatrix(radius);
    return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
}

void KisWaveletKernel::applyWavelet(KisPaintDeviceSP device,
                                      const QRect& rect,
                                      qreal xRadius, qreal yRadius,
                                      const QBitArray &channelFlags,
                                      KoUpdater *progressUpdater)
{
    QPoint srcTopLeft = rect.topLeft();

    if (xRadius > 0.0 && yRadius > 0.0) {
        KisPaintDeviceSP interm = new KisPaintDevice(device->colorSpace());
        interm->prepareClone(device);

        KisConvolutionKernelSP kernelHoriz = KisWaveletKernel::createHorizontalKernel(xRadius);
        KisConvolutionKernelSP kernelVertical = KisWaveletKernel::createVerticalKernel(yRadius);

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

        KisConvolutionKernelSP kernelHoriz = KisWaveletKernel::createHorizontalKernel(xRadius);
        painter.applyMatrix(kernelHoriz, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);

    } else if (yRadius > 0.0) {
        KisConvolutionPainter painter(device);
        painter.setChannelFlags(channelFlags);
        painter.setProgress(progressUpdater);

        KisConvolutionKernelSP kernelVertical = KisWaveletKernel::createVerticalKernel(yRadius);
        painter.applyMatrix(kernelVertical, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);
    }
}
