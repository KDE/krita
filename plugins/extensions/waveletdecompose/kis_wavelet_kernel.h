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

#ifndef __KIS_WAVELET_KERNEL_H
#define __KIS_WAVELET_KERNEL_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include <Eigen/Core>
using namespace Eigen;

class QRect;

class KRITAIMAGE_EXPORT KisWaveletKernel
{
public:
    static Matrix<qreal, Dynamic, Dynamic>
        createHorizontalMatrix(qreal radius);

    static Matrix<qreal, Dynamic, Dynamic>
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
