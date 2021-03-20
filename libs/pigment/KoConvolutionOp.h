/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
     * @param kernelValues the coefficient of the source pixels
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
