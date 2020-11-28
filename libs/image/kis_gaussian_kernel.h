/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
