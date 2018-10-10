/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#ifndef KO_MIX_COLORS_OP_H
#define KO_MIX_COLORS_OP_H

#include <limits.h>

/**
 * Base class of the mix color operation. It's defined by
 * sum(colors[i] * weights[i]) / 255. You access the KoMixColorsOp
 * of a colorspace by calling KoColorSpace::mixColorsOp.
 */
class KoMixColorsOp
{
public:
    virtual ~KoMixColorsOp() { }
    /**
     * Mix the colors.
     * @param colors a pointer toward the source pixels
     * @param weights the coefficient of the source pixels (if you want
     *                to average the sum of weights must be equal to 255)
     * @param nColors the number of pixels in the colors array
     * @param dst the destination pixel
     *
     * @code
     * quint8* colors[nColors];
     * colors[0] = ptrToFirstPixel;
     * colors[1] = ptrToSecondPixel;
     * ...
     * colors[nColors-1] = ptrToLastPixel;
     * qint16 weights[nColors];
     * weights[0] = firstWeight;
     * weights[1] = secondWeight;
     * ...
     * weights[nColors-1] = lastWeight;
     *
     * mixColors(colors, weights, nColors, ptrToDestinationPixel);
     * @endcode
     */
    virtual void mixColors(const quint8 * const*colors, const qint16 *weights, quint32 nColors, quint8 *dst) const = 0;
    virtual void mixColors(const quint8 *colors, const qint16 *weights, quint32 nColors, quint8 *dst) const = 0;


    /**
     * Mix the colors uniformly, without weightening
     * @param colors a pointer toward the source pixels
     * @param nColors the number of pixels in the colors array
     * @param dst the destination pixel
     *
     * @code
     * quint8* colors[nColors];
     * colors[0] = ptrToFirstPixel;
     * colors[1] = ptrToSecondPixel;
     * ...
     * colors[nColors-1] = ptrToLastPixel;
     *
     * mixColors(colors, nColors, ptrToDestinationPixel);
     * @endcode
     */
    virtual void mixColors(const quint8 * const*colors, quint32 nColors, quint8 *dst) const = 0;
    virtual void mixColors(const quint8 *colors, quint32 nColors, quint8 *dst) const = 0;
};

#endif
