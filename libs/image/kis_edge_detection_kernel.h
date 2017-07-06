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

#ifndef KIS_EDGE_DETECTION_KERNEL_H
#define KIS_EDGE_DETECTION_KERNEL_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include <Eigen/Core>

class QRect;

class KRITAIMAGE_EXPORT KisEdgeDetectionKernel
{
public:
    KisEdgeDetectionKernel();

    enum FilterType {
        Simple, //A weird simple method used in our old sobel filter
        Prewit, //The simpler prewitt detection, which doesn't smooth.
        SobolVector //Sobol does smooth. The creation of bigger kernels is based on an approach regarding vectors.
    };

    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
        createHorizontalMatrix(qreal radius, FilterType type);

    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
        createVerticalMatrix(qreal radius, FilterType type);

    static KisConvolutionKernelSP
        createHorizontalKernel(qreal radius, FilterType type);

    static KisConvolutionKernelSP
        createVerticalKernel(qreal radius, FilterType type);

    static int kernelSizeFromRadius(qreal radius);
    static qreal sigmaFromRadius(qreal radius);

    static void applyEdgeDetection(KisPaintDeviceSP device,
                              const QRect& rect,
                              qreal xRadius, qreal yRadius,
                              FilterType type,
                              const QBitArray &channelFlags,
                              KoUpdater *progressUpdater);
};

#endif // KIS_EDGE_DETECTION_KERNEL_H
