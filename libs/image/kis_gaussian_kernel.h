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

#ifndef __KIS_GAUSSIAN_KERNEL_H
#define __KIS_GAUSSIAN_KERNEL_H

#include "kritaimage_export.h"
#include "kis_types.h"
#include "kis_convolution_painter.h"

#include <Eigen/Core>

class QRect;

class KRITAIMAGE_EXPORT KisGaussianKernel
{
public:
    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
        createHorizontalMatrix(qreal radius);

    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
        createVerticalMatrix(qreal radius);

    static KisConvolutionKernelSP
        createHorizontalKernel(qreal radius);

    static KisConvolutionKernelSP
        createVerticalKernel(qreal radius);

    static KisConvolutionKernelSP
        createUniform2DKernel(qreal xRadius, qreal yRadius);

    static qreal sigmaFromRadius(qreal radius);
    static int kernelSizeFromRadius(qreal radius);

    static void applyGaussian(KisPaintDeviceSP device,
                              const QRect& rect,
                              qreal xRadius, qreal yRadius,
                              const QBitArray &channelFlags,
                              KoUpdater *updater,
                              bool createTransaction = false,
                              KisConvolutionBorderOp borderOp = BORDER_REPEAT);

    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> createLoGMatrix(qreal radius, qreal coeff, bool zeroCentered, bool includeWrappedArea);

    static void applyLoG(KisPaintDeviceSP device,
                         const QRect& rect,
                         qreal radius,
                         qreal coeff,
                         const QBitArray &channelFlags,
                         KoUpdater *progressUpdater);

    static void applyTightLoG(KisPaintDeviceSP device,
                              const QRect& rect,
                              qreal radius, qreal coeff,
                              const QBitArray &channelFlags,
                              KoUpdater *progressUpdater);


    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> createDilateMatrix(qreal radius);

    static void applyDilate(KisPaintDeviceSP device,
                            const QRect& rect,
                            qreal radius,
                            const QBitArray &channelFlags,
                            KoUpdater *progressUpdater,
                            bool createTransaction = false);

    static void applyErodeU8(KisPaintDeviceSP device,
                             const QRect& rect,
                             qreal radius,
                             const QBitArray &channelFlags,
                             KoUpdater *progressUpdater,
                             bool createTransaction = false);
};

#endif /* __KIS_GAUSSIAN_KERNEL_H */
