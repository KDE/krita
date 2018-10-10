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
        SobelVector //Sobel does smooth. The creation of bigger kernels is based on an approach regarding vectors.
    };

    enum FilterOutput {
        pythagorean,
        xGrowth,
        xFall,
        yGrowth,
        yFall,
        radian
    };

    /**
     * @brief createHorizontalMatrix
     * @param radius the radius. 1 makes a 3x3 kernel.
     * @param type One of the entries in the enum Filtertype
     * @param reverse which direction the gradient goes.
     * The horizontal gradient by default detects the rightmost edges.
     * Reversed it selects the leftmost edges.
     * @return
     */

    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
        createHorizontalMatrix(qreal radius, FilterType type, bool reverse = false);
    /**
     * @brief createVerticalMatrix
     * @param radius the radius. 1 makes a 3x3 kernel.
     * @param type One of the entries in the enum Filtertype
     * @param reverse which direction the gradient goes.
     * The vertical gradient by default detects the topmost edges.
     * Reversed it selects the bottommost edges.
     * @return
     */
    static Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>
        createVerticalMatrix(qreal radius, FilterType type, bool reverse = false);

    static KisConvolutionKernelSP
        createHorizontalKernel(qreal radius, FilterType type, bool denormalize = true, bool reverse = false);

    static KisConvolutionKernelSP
        createVerticalKernel(qreal radius, FilterType type, bool denormalize = true, bool reverse = false);

    static int kernelSizeFromRadius(qreal radius);
    static qreal sigmaFromRadius(qreal radius);

    /**
     * @brief applyEdgeDetection
     * This applies the edge detection filter to the device.
     * @param device the device to apply to.
     * @param rect the affected rect.
     * @param xRadius the radius of the horizontal sampling, radius of 0 is effectively disabling it.
     * @param yRadius the radius of the vertical sampling, refius of 0 is effectively disabling it.
     * @param type the type can be prewitt, sobel or simple, each of which
     * have a different sampling for the eventual edge detection.
     * @param channelFlags the affected channels.
     * @param progressUpdater the progress updater if it exists.
     * @param writeToAlpha whether or not to have the result applied to the transparency than the color channels,
     * this is useful for fringe effects.
     */
    static void applyEdgeDetection(KisPaintDeviceSP device,
                              const QRect& rect,
                              qreal xRadius, qreal yRadius,
                              FilterType type,
                              const QBitArray &channelFlags,
                              KoUpdater *progressUpdater,
                              FilterOutput output = pythagorean,
                              bool writeToAlpha = false);
    /**
     * @brief converToNormalMap
     * Convert a channel of the device to a normal map. The channel will be interpreted as a heightmap.
     * @param device the device
     * @param rect the rectangle to apply this to.
     * @param xRadius the xradius
     * @param yRadius the yradius
     * @param type the edge detection filter.
     * @param channelToConvert the channel to use as a grayscale.
     * @param channelOrder the order in which the xyz coordinates ought to be written to the pixels.
     * @param channelFlags
     * @param progressUpdater
     */
    static void convertToNormalMap(KisPaintDeviceSP device,
                                  const QRect & rect,
                                  qreal xRadius,
                                  qreal yRadius,
                                  FilterType type,
                                  int channelToConvert,
                                  QVector<int> channelOrder,
                                  QVector<bool> channelFlip,
                                  const QBitArray &channelFlags,
                                  KoUpdater *progressUpdater);
};

#endif // KIS_EDGE_DETECTION_KERNEL_H
