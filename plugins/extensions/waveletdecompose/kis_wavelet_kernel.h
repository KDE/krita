/*
 *  SPDX-FileCopyrightText: 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_WAVELET_KERNEL_H
#define __KIS_WAVELET_KERNEL_H

#include "kis_types.h"

#include <Eigen/Core>

class QRect;

class KisWaveletKernel
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

    static int kernelSizeFromRadius(qreal radius);

    static void applyWavelet(KisPaintDeviceSP device,
                              const QRect& rect,
                              qreal xRadius, qreal yRadius,
                              const QBitArray &channelFlags,
                              KoUpdater *updater);
};

#endif /* __KIS_WAVELET_KERNEL_H */
