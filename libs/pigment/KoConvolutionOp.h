/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KO_CONVOLUTION_OP_H
#define KO_CONVOLUTION_OP_H

/**
 * Base class of a convolution operation. A convolution operation is
 * defined by sum(colors[i] * kernelValues[i]) / factor + offset). The
 * most well known convolution is the gaussian blur.
 *
 * You access the KoConvolutionOp of a colorspace by calling
 * KoColorSpace::convolutionOp.
 */
class KoConvolutionOp
{
public:
    virtual ~KoConvolutionOp() { }
    /**
     * Convolve the colors.
     *
     * @param colors a pointer toward the source pixels
     * @param kernelValues the coeffient of the source pixels
     * @param dst the destination pixel
     * @param factor usually the factor is equal to the sum of kernelValues
     * @param offset the offset which is added to the result, useful
     *        when the sum of kernelValues is equal to 0
     * @param nColors the number of pixels in the colors array
     * @param channelFlags determines which channels are affected in pixel order
     *
     * This function is thread-safe.
     *
     */
    virtual void convolveColors(const quint8* const* colors, const qreal* kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nColors, const QBitArray & channelFlags) const = 0;
};

#endif 
