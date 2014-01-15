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
    const qreal center = 0.5 * kernelSize;

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
    const qreal center = 0.5 * kernelSize;

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
