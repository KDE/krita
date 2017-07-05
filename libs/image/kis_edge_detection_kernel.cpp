/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include "kis_edge_detection_kernel.h"
#include "kis_global.h"
#include "kis_convolution_kernel.h"
#include <kis_convolution_painter.h>
#include <QRect>

KisEdgeDetectionKernel::KisEdgeDetectionKernel()
{

}
/*
 * This code is very similar to the gaussian kernel code, except unlike the gaussian code,
 * edge-detection kernels DO use the diagonals.
 */

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> KisEdgeDetectionKernel::createHorizontalMatrix(qreal radius, FilterType type)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, kernelSize);

    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    if (type==Prewit) {
        for (int x = 0; x < kernelSize; x++) {
            for (int y=0; y<kernelSize; y++) {
                qreal xDistance = center - x;
                matrix(x, y) = xDistance;
            }
        }
    } else {
        for (int x = 0; x < kernelSize; x++) {
            for (int y=0; y<kernelSize; y++) {
                qreal xD = center - x;
                qreal yD = center - y;
                matrix(x, y) = xD / (xD*xD + yD*yD);
            }
        }
    }
    return matrix;
}

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> KisEdgeDetectionKernel::createVerticalMatrix(qreal radius, FilterType type)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, kernelSize);
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    if (type==Prewit) {
        for (int y = 0; y < kernelSize; y++) {
            for (int x=0; x<kernelSize; x++) {
                qreal yDistance = center - y;
                matrix(x, y) = yDistance;
            }
        }
    } else {
        for (int y = 0; y < kernelSize; y++) {
            for (int x=0; x<kernelSize; x++) {
                qreal xD = center - x;
                qreal yD = center - y;
                matrix(x, y) = yD / (xD*xD + yD*yD);
            }
        }
    }
    return matrix;
}

KisConvolutionKernelSP KisEdgeDetectionKernel::createHorizontalKernel(qreal radius, KisEdgeDetectionKernel::FilterType type)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createHorizontalMatrix(radius, type);
    return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
}

KisConvolutionKernelSP KisEdgeDetectionKernel::createVerticalKernel(qreal radius, KisEdgeDetectionKernel::FilterType type)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createVerticalMatrix(radius, type);
    return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
}

int KisEdgeDetectionKernel::kernelSizeFromRadius(qreal radius)
{
    return 6 * ceil(sigmaFromRadius(radius)) + 1;
}

qreal KisEdgeDetectionKernel::sigmaFromRadius(qreal radius)
{
    return 0.3 * radius + 0.3;
}

void KisEdgeDetectionKernel::applyEdgeDetection(KisPaintDeviceSP device,
                                                const QRect &rect,
                                                qreal xRadius,
                                                qreal yRadius,
                                                KisEdgeDetectionKernel::FilterType type,
                                                const QBitArray &channelFlags,
                                                KoUpdater *progressUpdater)
{
    QPoint srcTopLeft = rect.topLeft();

    if (xRadius > 0.0 && yRadius > 0.0) {
        KisPaintDeviceSP interm = new KisPaintDevice(device->colorSpace());

        KisConvolutionKernelSP kernelHoriz = KisEdgeDetectionKernel::createHorizontalKernel(xRadius, type);
        KisConvolutionKernelSP kernelVertical = KisEdgeDetectionKernel::createVerticalKernel(yRadius, type);

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

        KisConvolutionKernelSP kernelHoriz = KisEdgeDetectionKernel::createHorizontalKernel(xRadius, type);
        painter.applyMatrix(kernelHoriz, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);

    } else if (yRadius > 0.0) {
        KisConvolutionPainter painter(device);
        painter.setChannelFlags(channelFlags);
        painter.setProgress(progressUpdater);

        KisConvolutionKernelSP kernelVertical = KisEdgeDetectionKernel::createVerticalKernel(yRadius, type);
        painter.applyMatrix(kernelVertical, device, srcTopLeft, srcTopLeft, rect.size(), BORDER_REPEAT);
    }
}
