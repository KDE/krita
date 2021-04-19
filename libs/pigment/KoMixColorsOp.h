/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KO_MIX_COLORS_OP_H
#define KO_MIX_COLORS_OP_H

#include <limits.h>

/**
 * Base class of the mix color operation. It's defined by
 * sum(colors[i] * weights[i]) / weightSum. You access the KoMixColorsOp
 * of a colorspace by calling KoColorSpace::mixColorsOp.
 */
class KoMixColorsOp
{
public:
    class Mixer
    {
    public:
        virtual ~Mixer() {}
        virtual void accumulate(const quint8 *data, const qint16 *weights, int weightSum, int nPixels) = 0;
        virtual void accumulateAverage(const quint8 *data, int nPixels) = 0;
        virtual void computeMixedColor(quint8 *data) = 0;
        virtual qint64 currentWeightsSum() const = 0;
    };

    virtual Mixer* createMixer() const = 0;

public:
    virtual ~KoMixColorsOp() { }
    /**
     * Mix the colors.
     * @param colors a pointer toward the source pixels
     * @param weights the coefficient of the source pixels
     * @param nColors the number of pixels in the colors array
     * @param dst the destination pixel
     * @param weightSum an integer representing the sum of the coefficients.
     *                  by default 255. If for some reason you do not want a
     *                  perfect avarage, make this anything but the sum. Try
     *                  to keep this below 255 for division-related performance.
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
    virtual void mixColors(const quint8 * const*colors, const qint16 *weights, int nColors, quint8 *dst, int weightSum = 255) const = 0;
    virtual void mixColors(const quint8 *colors, const qint16 *weights, int nColors, quint8 *dst, int weightSum = 255) const = 0;


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
    virtual void mixColors(const quint8 * const*colors, int nColors, quint8 *dst) const = 0;
    virtual void mixColors(const quint8 *colors, int nColors, quint8 *dst) const = 0;

    /**
     *   Convenience function to mix two color arrays with one weight.  Mixes colorsA[x] with colorsB[x] with 
     *   weight as the percentage of B vs A (0.0 -> 100% A, 1.0 -> 100% B), for all x = [0 .. nColors-1].
     *
     *
    */
    virtual void mixTwoColorArrays(const quint8* colorsA, const quint8* colorsB, int nColors, qreal weight, quint8* dst) const = 0;

    /**
     *   Convenience function to mix one color array with one color with a specific weight.  Mixes colorArray[x] with color with
     *   weight as the percentage of B vs A (0.0 -> 100% A, 1.0 -> 100% B), for all x = [0 .. nColors-1].
     *
     *
    */
    virtual void mixArrayWithColor(const quint8* colorArray, const quint8* color, int nColors, qreal weight, quint8* dst) const = 0;
};

#endif
