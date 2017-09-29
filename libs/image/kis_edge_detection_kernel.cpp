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
#include <KoCompositeOpRegistry.h>
#include <QRect>
#include <KoColorSpace.h>
#include <kis_iterator_ng.h>

KisEdgeDetectionKernel::KisEdgeDetectionKernel()
{

}
/*
 * This code is very similar to the gaussian kernel code, except unlike the gaussian code,
 * edge-detection kernels DO use the diagonals.
 * Except for the simple mode. We implement the simple mode because it is an analogue to
 * the old sobel filter.
 */

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> KisEdgeDetectionKernel::createHorizontalMatrix(qreal radius,
                                                                                                    FilterType type,
                                                                                                    bool reverse)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, kernelSize);

    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    if (type==Prewit) {
        for (int x = 0; x < kernelSize; x++) {
            for (int y=0; y<kernelSize; y++) {
                qreal xDistance;
                if (reverse) {
                    xDistance = x - center;
                } else {
                    xDistance = center - x;
                }
                matrix(x, y) = xDistance;
            }
        }
    } else if(type==Simple) {
        matrix.resize(kernelSize, 1);
        for (int x = 0; x < kernelSize; x++) {
            qreal xDistance;
            if (reverse) {
                xDistance = x - center;
            } else {
                xDistance = center - x;
            }
            if (x==center) {
                matrix(x, 0) = 0;
            } else {
                matrix(x, 0) = (1/xDistance);
            }
        }
    } else {
        for (int x = 0; x < kernelSize; x++) {
            for (int y=0; y<kernelSize; y++) {
                if (x==center && y==center) {
                    matrix(x, y) = 0;
                } else {
                    qreal xD, yD;
                    if (reverse) {
                        xD = x - center;
                        yD = y - center;
                    } else {
                        xD = center - x;
                        yD = center - y;
                    }
                    matrix(x, y) = xD / (xD*xD + yD*yD);
                }
            }
        }
    }
    return matrix;
}

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> KisEdgeDetectionKernel::createVerticalMatrix(qreal radius,
                                                                                                  FilterType type,
                                                                                                  bool reverse)
{
    int kernelSize = kernelSizeFromRadius(radius);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, kernelSize);
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    if (type==Prewit) {
        for (int y = 0; y < kernelSize; y++) {
            for (int x=0; x<kernelSize; x++) {
                qreal yDistance;
                if (reverse) {
                    yDistance = y - center;
                } else {
                    yDistance = center - y;
                }
                matrix(x, y) = yDistance;
            }
        }
    } else if(type==Simple) {
        matrix.resize(1, kernelSize);
        for (int y = 0; y < kernelSize; y++) {
            qreal yDistance;
            if (reverse) {
                yDistance = y - center;
            } else {
                yDistance = center - y;
            }
            if (y==center) {
                matrix(0, y) = 0;
            } else {
                matrix(0, y) = (1/yDistance);
            }
        }
    } else {
        for (int y = 0; y < kernelSize; y++) {
            for (int x=0; x<kernelSize; x++) {
                if (x==center && y==center) {
                    matrix(x, y) = 0;
                } else {
                    qreal xD, yD;
                    if (reverse) {
                        xD = x - center;
                        yD = y - center;
                    } else {
                        xD = center - x;
                        yD = center - y;
                    }
                    matrix(x, y) = yD / (xD*xD + yD*yD);
                }
            }
        }
    }
    return matrix;
}

KisConvolutionKernelSP KisEdgeDetectionKernel::createHorizontalKernel(qreal radius,
                                                                      KisEdgeDetectionKernel::FilterType type,
                                                                      bool reverse)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createHorizontalMatrix(radius, type, reverse);
    return KisConvolutionKernel::fromMatrix(matrix, 0.5, 1);
}

KisConvolutionKernelSP KisEdgeDetectionKernel::createVerticalKernel(qreal radius,
                                                                    KisEdgeDetectionKernel::FilterType type,
                                                                    bool reverse)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createVerticalMatrix(radius, type, reverse);
    return KisConvolutionKernel::fromMatrix(matrix, 0.5, 1);
}

int KisEdgeDetectionKernel::kernelSizeFromRadius(qreal radius)
{
    return qMax((int)(2 * ceil(sigmaFromRadius(radius)) + 1), 3);
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
    KisPainter finalPainter(device);
    finalPainter.setChannelFlags(channelFlags);
    finalPainter.setProgress(progressUpdater);

    if (xRadius > 0.0) {
        KisPaintDeviceSP x_denormalised = new KisPaintDevice(device->colorSpace());

        KisConvolutionKernelSP kernelHorizLeftRight = KisEdgeDetectionKernel::createHorizontalKernel(xRadius, type);

        qreal verticalCenter = qreal(kernelHorizLeftRight->width()) / 2.0;

        KisConvolutionPainter horizPainterLR(x_denormalised);
        horizPainterLR.setChannelFlags(channelFlags);
        horizPainterLR.setProgress(progressUpdater);
        horizPainterLR.applyMatrix(kernelHorizLeftRight, device,
                                 srcTopLeft - QPoint(0, ceil(verticalCenter)),
                                 srcTopLeft - QPoint(0, ceil(verticalCenter)),
                                 rect.size() + QSize(0, 2 * ceil(verticalCenter)), BORDER_REPEAT);
        finalPainter.bitBlt(srcTopLeft, x_denormalised, rect);
    }
    if (yRadius > 0.0) {
        KisPaintDeviceSP y_denormalised = new KisPaintDevice(device->colorSpace());
        KisConvolutionKernelSP kernelVerticalTopBottom = KisEdgeDetectionKernel::createVerticalKernel(yRadius, type);
        qreal verticalCenter = qreal(kernelVerticalTopBottom->height()) / 2.0;

        KisConvolutionPainter verticalPainterTB(y_denormalised);
        verticalPainterTB.setChannelFlags(channelFlags);
        verticalPainterTB.setProgress(progressUpdater);
        verticalPainterTB.applyMatrix(kernelVerticalTopBottom, device,
                                 srcTopLeft - QPoint(0, ceil(verticalCenter)),
                                 srcTopLeft - QPoint(0, ceil(verticalCenter)),
                                 rect.size() + QSize(0, 2 * ceil(verticalCenter)), BORDER_REPEAT);
        if (xRadius > 0.0) {
            KisSequentialIterator yItterator(y_denormalised, rect);
            KisSequentialIterator xItterator(device, rect);
            const int pixelSize = device->colorSpace()->pixelSize();
            do {

                    QVector<float> yNormalised(device->channelCount());
                    device->colorSpace()->normalisedChannelsValue(yItterator.rawData(), yNormalised);
                    QVector<float> xNormalised(device->channelCount());
                    device->colorSpace()->normalisedChannelsValue(xItterator.rawData(), xNormalised);
                    for (quint32 c = 0; c<device->channelCount(); c++) {
                        xNormalised[c] = 2 * sqrt( ((xNormalised[c]-0.5)*(xNormalised[c]-0.5)) + ((yNormalised[c]-0.5)*(yNormalised[c]-0.5)));
                    }
                    quint8* f = xItterator.rawData();
                    device->colorSpace()->fromNormalisedChannelsValue(f, xNormalised);
                    memcpy(xItterator.rawData(), f, pixelSize);
            } while(yItterator.nextPixel() && xItterator.nextPixel());

        } else {
            finalPainter.bitBlt(srcTopLeft, y_denormalised, rect);
        }
    }
}
