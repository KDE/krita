/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
 *
 */

#include <KoColorSpace.h>
#include <KoIntegerMaths.h>

#include "kis_rgb_u8_compositeop.h"
#include "kis_rgb_colorspace.h"
#include "KoColorConversions.h"

#define PixelIntensity(pixel) ((unsigned int)                           \
                               (((double)306.0 * (pixel[RgbU8Traits::red_pos]) + \
                                 (double)601.0 * (pixel[RgbU8Traits::green_pos]) + \
                                 (double)117.0 * (pixel[RgbU8Traits::blue_pos)) \
                                 / 1024.0))

#define PixelIntensityToQuantum(pixel) ((quint8)PixelIntensity(pixel))

#define PixelIntensityToDouble(pixel) ((double)PixelIntensity(pixel))

#define RoundSignedToQuantum(value) ((quint8) (value < 0 ? 0 :          \
                                               (value > UINT8_MAX) ? UINT8_MAX : value + 0.5))

#define RoundToQuantum(value) ((quint8) (value > UINT8_MAX ? UINT8_MAX : \
                                         value + 0.5))

// And from studio.h
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))


KisRgbU8CompositeOp::KisRgbU8CompositeOp(KoColorSpace * cs, const QString& id, const QString& description, const bool userVisible)
    : KoCompositeOp(cs, id, description, userVisible)
{
    m_pixelSize = cs->pixelSize();
}


void KisRgbU8CompositeOp::composite(quint8 *dst, qint32 dstRowStride,
                                    const quint8 *src, qint32 srcRowStride,
                                    const quint8 *mask, qint32 maskRowStride,
                                    qint32 rows, qint32 cols,
                                    quint8 opacity,
                                    const QBitArray & channelFlags) const
{
    Q_ASSERT( channelFlags.size() == 4 || channelFlags.isEmpty() );

    if ( m_id == COMPOSITE_UNDEF ) {
        // Undefined == no composition
    }
    else if ( m_id == COMPOSITE_OVER ) {
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( m_id == COMPOSITE_IN ) {
        compositeIn(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( m_id == COMPOSITE_OUT ) {
        compositeOut(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( m_id == COMPOSITE_ATOP ) {
        compositeAtop(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( m_id == COMPOSITE_XOR ) {
        compositeXor(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( m_id == COMPOSITE_PLUS ) {
        compositePlus(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_MINUS) {
        compositeMinus(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_ADD) {
        compositeAdd(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_SUBTRACT) {
        compositeSubtract(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_DIFF) {
        compositeDiff(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_MULT) {
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_DIVIDE) {
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_BUMPMAP) {
        compositeBumpmap(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_COPY) {
        compositeCopy(m_pixelSize, dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_COPY_RED) {
        compositeCopyRed(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_COPY_GREEN) {
        compositeCopyGreen(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_COPY_BLUE) {
        compositeCopyBlue(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_COPY_OPACITY) {
        compositeCopyOpacity(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_CLEAR) {
        compositeClear(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_DISSOLVE) {
        compositeDissolve(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_NO) {
        // No composition.
    }
    else if (m_id == COMPOSITE_DARKEN) {
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_LIGHTEN) {
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_HUE) {
        compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_SATURATION) {
        compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_VALUE) {
        compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_COLOR) {
        compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_SCREEN) {
        compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_OVERLAY) {
        compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_ERASE) {
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_DODGE) {
        compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (m_id == COMPOSITE_BURN) {
        compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }

}




void KisRgbU8CompositeOp::compositeOver(quint8 *dstRowStart, qint32 dstRowStride,
                                        const quint8 *srcRowStart, qint32 srcRowStride,
                                        const quint8 *maskRowStart, qint32 maskRowStride,
                                        qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, RgbU8Traits::channels_nb * sizeof(quint8));
                } else {
                    quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

                    quint8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                             dst[RgbU8Traits::alpha_pos] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE ) {
                        if ( channelFlags.isEmpty() )
                            memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(quint8));
                        else {
                            if ( channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                                dst[RgbU8Traits::red_pos] = src[RgbU8Traits::red_pos];
                            }
                            if ( channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                                dst[RgbU8Traits::green_pos] = src[RgbU8Traits::green_pos];
                            }
                            if ( channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                                dst[RgbU8Traits::blue_pos] = src[RgbU8Traits::blue_pos];
                            }
                        }
                    } else {
                        if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                             dst[RgbU8Traits::red_pos] = UINT8_BLEND(src[RgbU8Traits::red_pos], dst[RgbU8Traits::red_pos], srcBlend);
                        if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                            dst[RgbU8Traits::green_pos] = UINT8_BLEND(src[RgbU8Traits::green_pos], dst[RgbU8Traits::green_pos], srcBlend);
                        if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                            dst[RgbU8Traits::blue_pos] = UINT8_BLEND(src[RgbU8Traits::blue_pos], dst[RgbU8Traits::blue_pos], srcBlend);
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }


            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                        dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                quint8 srcColor;
                quint8 dstColor;
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {

                    quint8 srcColor = src[RgbU8Traits::red_pos];
                    quint8 dstColor = dst[RgbU8Traits::red_pos];

                    srcColor = UINT8_MULT(srcColor, dstColor);

                    dst[RgbU8Traits::red_pos] = UINT8_BLEND(srcColor, dstColor, srcBlend);
                }
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                    srcColor = src[RgbU8Traits::green_pos];
                    dstColor = dst[RgbU8Traits::green_pos];

                    srcColor = UINT8_MULT(srcColor, dstColor);

                    dst[RgbU8Traits::green_pos] = UINT8_BLEND(srcColor, dstColor, srcBlend);
                }
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                    srcColor = src[RgbU8Traits::blue_pos];
                    dstColor = dst[RgbU8Traits::blue_pos];

                    srcColor = UINT8_MULT(srcColor, dstColor);

                    dst[RgbU8Traits::blue_pos] = UINT8_BLEND(srcColor, dstColor, srcBlend);
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    if ( channelFlags.isEmpty() || channelFlags.testBit( channel ) ) {

                             quint8 srcColor = src[channel];
                             quint8 dstColor = dst[channel];

                             srcColor = qMin((dstColor * (UINT8_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT8_MAX);

                             quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                             dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if ( channelFlags.isEmpty() || channelFlags.testBit( channel ) ) {

                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }


            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if ( channelFlags.isEmpty() || channelFlags.testBit( channel ) ) {

                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }


            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = qMin((dstColor * (UINT8_MAX + 1)) / (UINT8_MAX + 1 - srcColor), UINT8_MAX);

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = qMin(((UINT8_MAX - dstColor) * (UINT8_MAX + 1)) / (srcColor + 1), UINT8_MAX);
                        if (UINT8_MAX - srcColor > UINT8_MAX) srcColor = UINT8_MAX;

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = qMin(srcColor, dstColor);

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = qMax(srcColor, dstColor);

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeHue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[RgbU8Traits::red_pos];
                int dstGreen = dst[RgbU8Traits::green_pos];
                int dstBlue = dst[RgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[RgbU8Traits::red_pos], src[RgbU8Traits::green_pos], src[RgbU8Traits::blue_pos], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                    dst[RgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                    dst[RgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                    dst[RgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeSaturation(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[RgbU8Traits::red_pos];
                int dstGreen = dst[RgbU8Traits::green_pos];
                int dstBlue = dst[RgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[RgbU8Traits::red_pos], src[RgbU8Traits::green_pos], src[RgbU8Traits::blue_pos], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                    dst[RgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                    dst[RgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                    dst[RgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeValue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[RgbU8Traits::red_pos];
                int dstGreen = dst[RgbU8Traits::green_pos];
                int dstBlue = dst[RgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[RgbU8Traits::red_pos], src[RgbU8Traits::green_pos], src[RgbU8Traits::blue_pos], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                    dst[RgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                    dst[RgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                    dst[RgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeColor(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
{

    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[RgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[RgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[RgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[RgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[RgbU8Traits::red_pos];
                int dstGreen = dst[RgbU8Traits::green_pos];
                int dstBlue = dst[RgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcLightness;
                int dstHue;
                int dstSaturation;
                int dstLightness;

                rgb_to_hls(src[RgbU8Traits::red_pos], src[RgbU8Traits::green_pos], src[RgbU8Traits::blue_pos], &srcHue, &srcLightness, &srcSaturation);
                rgb_to_hls(dstRed, dstGreen, dstBlue, &dstHue, &dstLightness, &dstSaturation);

                quint8 srcRed;
                quint8 srcGreen;
                quint8 srcBlue;

                hls_to_rgb(srcHue, dstLightness, srcSaturation, &srcRed, &srcGreen, &srcBlue);

                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                    dst[RgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                    dst[RgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                    dst[RgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += RgbU8Traits::channels_nb;
            dst += RgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }

}

void KisRgbU8CompositeOp::compositeErase(quint8 *dst,
                                         qint32 dstRowSize,
                                         const quint8 *src,
                                         qint32 srcRowSize,
                                         const quint8 *srcAlphaMask,
                                         qint32 maskRowStride,
                                         qint32 rows,
                                         qint32 cols,
                                         quint8 opacity,
                                         const QBitArray & channelFlags) const
{
    Q_UNUSED( opacity );
    Q_UNUSED( channelFlags );

    qint32 i;
    quint8 srcAlpha;
    while (rows-- > 0)
    {
        const quint8 *s = src;
        quint8 *d = dst;
        const quint8 *mask = srcAlphaMask;

        for (i = cols; i > 0; i--, s+=RgbU8Traits::channels_nb, d+=RgbU8Traits::channels_nb)
        {
            srcAlpha = s[RgbU8Traits::alpha_pos];
            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, *mask);
                mask++;
            }
            d[RgbU8Traits::alpha_pos] = UINT8_MULT(srcAlpha, d[RgbU8Traits::alpha_pos]);
        }

        dst += dstRowSize;
        if(srcAlphaMask)
            srcAlphaMask += maskRowStride;
        src += srcRowSize;
    }

}



void KisRgbU8CompositeOp::compositeIn(qint32 pixelSize,
                                      quint8 *dst,
                                      qint32 dstRowSize,
                                      const quint8 *src,
                                      qint32 srcRowSize,
                                      qint32 rows,
                                      qint32 cols,
                                      quint8 opacity, const QBitArray & channelFlags) const
{

    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double alpha;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {

            if (s[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
            {
                memcpy(d, s, pixelSize * sizeof(quint8));
                continue;
            }
            if (d[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
                continue;

            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            alpha=(double) (((double) UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) / UINT8_MAX);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8) (((double) UINT8_MAX - sAlpha) *
                                                  (UINT8_MAX-dAlpha) * s[RgbU8Traits::red_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]=(quint8) (((double) UINT8_MAX - sAlpha)*
                                                    (UINT8_MAX-dAlpha) * s[RgbU8Traits::green_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos]=(quint8) (((double) UINT8_MAX - sAlpha)*
                                                   (UINT8_MAX - dAlpha) * s[RgbU8Traits::blue_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=(quint8) ((d[RgbU8Traits::alpha_pos] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);

        }
        dst += dstRowSize;
        src += srcRowSize;
    }
}

void KisRgbU8CompositeOp::compositeOut(qint32 pixelSize,
                                       quint8 *dst,
                                       qint32 dstRowSize,
                                       const quint8 *src,
                                       qint32 srcRowSize,
                                       qint32 rows,
                                       qint32 cols,
                                       quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double alpha;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            if (s[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
            {
                memcpy(d, s, pixelSize * sizeof(quint8));
                break;
            }
            if (d[RgbU8Traits::alpha_pos] == OPACITY_OPAQUE)
            {
                d[RgbU8Traits::alpha_pos]=OPACITY_TRANSPARENT;
                break;
            }
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            alpha=(double) (UINT8_MAX - sAlpha) * d[RgbU8Traits::alpha_pos]/UINT8_MAX;
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos] = (quint8) (((double) UINT8_MAX - sAlpha) * dAlpha * s[RgbU8Traits::red_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos] = (quint8) (((double) UINT8_MAX - sAlpha) * dAlpha * s[RgbU8Traits::green_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos] = (quint8) (((double) UINT8_MAX - sAlpha) * dAlpha * s[RgbU8Traits::blue_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=(quint8) ((d[RgbU8Traits::alpha_pos] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}

void KisRgbU8CompositeOp::compositeAtop(qint32 pixelSize,
                                        quint8 *dst,
                                        qint32 dstRowSize,
                                        const quint8 *src,
                                        qint32 srcRowSize,
                                        qint32 rows,
                                        qint32 cols,
                                        quint8 opacity, const QBitArray & channelFlags) const
{

    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            alpha = ((double)(UINT8_MAX - sAlpha) *
                     (UINT8_MAX - dAlpha) + (double) sAlpha *
                     (UINT8_MAX - dAlpha)) / UINT8_MAX;

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red = ((double)(UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) *  s[RgbU8Traits::red_pos] / UINT8_MAX +
                       (double) sAlpha * (UINT8_MAX-dAlpha) * d[RgbU8Traits::red_pos]/UINT8_MAX) / alpha;
                d[RgbU8Traits::red_pos] = (quint8) (red > UINT8_MAX ? UINT8_MAX : red + 0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green = ((double) (UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) * s[RgbU8Traits::green_pos] / UINT8_MAX +
                         (double) sAlpha * (UINT8_MAX-dAlpha) * d[RgbU8Traits::green_pos]/UINT8_MAX)/alpha;
                d[RgbU8Traits::green_pos] = (quint8) (green > UINT8_MAX ? UINT8_MAX : green + 0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue = ((double) (UINT8_MAX - sAlpha) * (UINT8_MAX- dAlpha) * s[RgbU8Traits::blue_pos] / UINT8_MAX +
                        (double) sAlpha * (UINT8_MAX - dAlpha) * d[RgbU8Traits::blue_pos]/UINT8_MAX) / alpha;
                d[RgbU8Traits::blue_pos] = (quint8) (blue > UINT8_MAX ? UINT8_MAX : blue + 0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=(quint8) (UINT8_MAX - (alpha > UINT8_MAX ? UINT8_MAX : alpha) + 0.5);
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}


void KisRgbU8CompositeOp::compositeXor(qint32 pixelSize,
                                       quint8 *dst,
                                       qint32 dstRowSize,
                                       const quint8 *src,
                                       qint32 srcRowSize,
                                       qint32 rows,
                                       qint32 cols,
                                       quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            alpha =((double) (UINT8_MAX -sAlpha)*
                    dAlpha+(double) (UINT8_MAX -dAlpha)*
                    sAlpha)/UINT8_MAX ;
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=((double) (UINT8_MAX -sAlpha)*dAlpha*
                     s[RgbU8Traits::red_pos]/UINT8_MAX +(double) (UINT8_MAX -dAlpha)*
                     sAlpha*d[RgbU8Traits::red_pos]/UINT8_MAX )/alpha ;
                d[RgbU8Traits::red_pos]=RoundSignedToQuantum(Qt::red);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=((double) (UINT8_MAX -sAlpha)*dAlpha*
                       s[RgbU8Traits::green_pos]/UINT8_MAX +(double) (UINT8_MAX -dAlpha)*
                       sAlpha*d[RgbU8Traits::green_pos]/UINT8_MAX )/alpha ;
                d[RgbU8Traits::green_pos]=RoundSignedToQuantum(Qt::green);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=((double) (UINT8_MAX -sAlpha)*dAlpha*
                      s[RgbU8Traits::blue_pos]/UINT8_MAX +(double) (UINT8_MAX -dAlpha)*
                      sAlpha*d[RgbU8Traits::blue_pos]/UINT8_MAX )/alpha ;
                d[RgbU8Traits::blue_pos]=RoundSignedToQuantum(Qt::blue);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=UINT8_MAX -RoundSignedToQuantum(alpha );
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}


void KisRgbU8CompositeOp::compositePlus(qint32 pixelSize,
                                        quint8 *dst,
                                        qint32 dstRowSize,
                                        const quint8 *src,
                                        qint32 srcRowSize,
                                        qint32 rows,
                                        qint32 cols,
                                        quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=((double) (UINT8_MAX -sAlpha)*s[RgbU8Traits::red_pos]+(double)
                     (UINT8_MAX -dAlpha)*d[RgbU8Traits::red_pos])/UINT8_MAX ;
                d[RgbU8Traits::red_pos]=RoundSignedToQuantum(Qt::red);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=((double) (UINT8_MAX -sAlpha)*s[RgbU8Traits::green_pos]+(double)
                       (UINT8_MAX -dAlpha)*d[RgbU8Traits::green_pos])/UINT8_MAX ;
                d[RgbU8Traits::green_pos]=RoundSignedToQuantum(Qt::green);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=((double) (UINT8_MAX -sAlpha)*s[RgbU8Traits::blue_pos]+(double)
                      (UINT8_MAX -dAlpha)*d[RgbU8Traits::blue_pos])/UINT8_MAX ;
                d[RgbU8Traits::blue_pos]=RoundSignedToQuantum(Qt::blue);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                alpha =((double) (UINT8_MAX -sAlpha)+
                       (double) (UINT8_MAX -dAlpha))/UINT8_MAX ;
                d[RgbU8Traits::alpha_pos]=UINT8_MAX -RoundSignedToQuantum(alpha );
            }
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}



void KisRgbU8CompositeOp::compositeMinus(qint32 pixelSize,
                                         quint8 *dst,
                                         qint32 dstRowSize,
                                         const quint8 *src,
                                         qint32 srcRowSize,
                                         qint32 rows,
                                         qint32 cols,
                                         quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=((double) (UINT8_MAX -dAlpha)*d[RgbU8Traits::red_pos]-
                     (double) (UINT8_MAX -sAlpha)*s[RgbU8Traits::red_pos])/UINT8_MAX ;
                d[RgbU8Traits::red_pos]=RoundSignedToQuantum(Qt::red);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=((double) (UINT8_MAX -dAlpha)*d[RgbU8Traits::green_pos]-
                       (double) (UINT8_MAX -sAlpha)*s[RgbU8Traits::green_pos])/UINT8_MAX ;
                d[RgbU8Traits::green_pos]=RoundSignedToQuantum(Qt::green);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=((double) (UINT8_MAX -dAlpha)*d[RgbU8Traits::blue_pos]-
                      (double) (UINT8_MAX -sAlpha)*s[RgbU8Traits::blue_pos])/UINT8_MAX ;
                d[RgbU8Traits::blue_pos]=RoundSignedToQuantum(Qt::blue);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                alpha =((double) (UINT8_MAX -dAlpha)-
                        (double) (UINT8_MAX -sAlpha))/UINT8_MAX ;
                d[RgbU8Traits::alpha_pos]=UINT8_MAX -RoundSignedToQuantum(alpha );
            }
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}

void KisRgbU8CompositeOp::compositeAdd(qint32 pixelSize,
                                       quint8 *dst,
                                       qint32 dstRowSize,
                                       const quint8 *src,
                                       qint32 srcRowSize,
                                       qint32 rows,
                                       qint32 cols,
                                       quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;
    double red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=(double) s[RgbU8Traits::red_pos]+d[RgbU8Traits::red_pos];
                d[RgbU8Traits::red_pos]=(quint8)
                                        (red > UINT8_MAX  ? red-=UINT8_MAX  : red+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=(double) s[RgbU8Traits::green_pos]+d[RgbU8Traits::green_pos];
                d[RgbU8Traits::green_pos]=(quint8)
                                          (green > UINT8_MAX  ? green-=UINT8_MAX  : green+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=(double) s[RgbU8Traits::blue_pos]+d[RgbU8Traits::blue_pos];
                d[RgbU8Traits::blue_pos]=(quint8)
                                         (blue > UINT8_MAX  ? blue-=UINT8_MAX  : blue+0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=OPACITY_OPAQUE;
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}

void KisRgbU8CompositeOp::compositeSubtract(qint32 pixelSize,
                                            quint8 *dst,
                                            qint32 dstRowSize,
                                            const quint8 *src,
                                            qint32 srcRowSize,
                                            qint32 rows,
                                            qint32 cols,
                                            quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=(double) s[RgbU8Traits::red_pos]-d[RgbU8Traits::red_pos];
                d[RgbU8Traits::red_pos]=(quint8)
                                        (red < 0 ? red+=UINT8_MAX  : red+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=(double) s[RgbU8Traits::green_pos]-d[RgbU8Traits::green_pos];
                d[RgbU8Traits::green_pos]=(quint8)
                                          (green < 0 ? green+=UINT8_MAX  : green+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=(double) s[RgbU8Traits::blue_pos]-d[RgbU8Traits::blue_pos];
                d[RgbU8Traits::blue_pos]=(quint8)
                                         (blue < 0 ? blue+=UINT8_MAX  : blue+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=OPACITY_OPAQUE;

        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}

void KisRgbU8CompositeOp::compositeDiff(qint32 pixelSize,
                                        quint8 *dst,
                                        qint32 dstRowSize,
                                        const quint8 *src,
                                        qint32 srcRowSize,
                                        qint32 rows,
                                        qint32 cols,
                                        quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8)
                                        AbsoluteValue(s[RgbU8Traits::red_pos]-(double) d[RgbU8Traits::red_pos]);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]=(quint8)
                                          AbsoluteValue(s[RgbU8Traits::green_pos]-(double) d[RgbU8Traits::green_pos]);


            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos]=(quint8)
                                         AbsoluteValue(s[RgbU8Traits::blue_pos]-(double) d[RgbU8Traits::blue_pos]);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=UINT8_MAX - (quint8)
                                          AbsoluteValue(sAlpha-(double) dAlpha);

        }
        dst += dstRowSize;
        src += srcRowSize;

    }
}

void KisRgbU8CompositeOp::compositeBumpmap(qint32 pixelSize,
                                           quint8 *dst,
                                           qint32 dstRowSize,
                                           const quint8 *src,
                                           qint32 srcRowSize,
                                           qint32 rows,
                                           qint32 cols,
                                           quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double intensity;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            // Is this correct? It's not this way in GM.
            if (s[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
                continue;

            // And I'm not sure whether this is correct, either.
            intensity = ((double)306.0 * s[RgbU8Traits::red_pos] +
                         (double)601.0 * s[RgbU8Traits::green_pos] +
                         (double)117.0 * s[RgbU8Traits::blue_pos]) / 1024.0;

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8) (((double)
                                                   intensity * d[RgbU8Traits::red_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]=(quint8) (((double)
                                                     intensity * d[RgbU8Traits::green_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos]=(quint8) (((double)
                                                    intensity * d[RgbU8Traits::blue_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]= (quint8) (((double)
                                                      intensity * d[RgbU8Traits::alpha_pos])/UINT8_MAX +0.5);


        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}


void KisRgbU8CompositeOp::compositeCopyChannel(quint8 pixel, quint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity, const QBitArray & channelFlags) const
{
    Q_UNUSED( channelFlags );
    Q_UNUSED( opacity );
    quint8 *d;
    const quint8 *s;
    qint32 i;

    while (rows-- > 0) {
        d = dst;
        s = src;

        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            d[pixel] = s[pixel];
        }

        dst += dstRowSize;
        src += srcRowSize;
    }
}

void KisRgbU8CompositeOp::compositeCopyRed(qint32 pixelSize,
                                           quint8 *dst,
                                           qint32 dstRowSize,
                                           const quint8 *src,
                                           qint32 srcRowSize,
                                           qint32 rows,
                                           qint32 cols,
                                           quint8 opacity, const QBitArray & channelFlags) const
{
    compositeCopyChannel(RgbU8Traits::red_pos, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity, channelFlags);
}

void KisRgbU8CompositeOp::compositeCopyGreen(qint32 pixelSize,
                                             quint8 *dst,
                                             qint32 dstRowSize,
                                             const quint8 *src,
                                             qint32 srcRowSize,
                                             qint32 rows,
                                             qint32 cols,
                                             quint8 opacity, const QBitArray & channelFlags) const
{
    compositeCopyChannel(RgbU8Traits::green_pos, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity, channelFlags);
}

void KisRgbU8CompositeOp::compositeCopyBlue(qint32 pixelSize,
                                            quint8 *dst,
                                            qint32 dstRowSize,
                                            const quint8 *src,
                                            qint32 srcRowSize,
                                            qint32 rows,
                                            qint32 cols,
                                            quint8 opacity, const QBitArray & channelFlags) const
{
    compositeCopyChannel(RgbU8Traits::blue_pos, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity, channelFlags);
}


void KisRgbU8CompositeOp::compositeCopyOpacity(qint32 pixelSize,
                                               quint8 *dst,
                                               qint32 dstRowSize,
                                               const quint8 *src,
                                               qint32 srcRowSize,
                                               qint32 rows,
                                               qint32 cols,
                                               quint8 opacity, const QBitArray & channelFlags) const
{

    // XXX: mess with intensity if there isn't an alpha channel, according to GM.
    compositeCopyChannel(RgbU8Traits::alpha_pos, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity, channelFlags);

}


void KisRgbU8CompositeOp::compositeClear(qint32 pixelSize,
                                         quint8 *dst,
                                         qint32 dstRowSize,
                                         const quint8 *src,
                                         qint32 srcRowSize,
                                         qint32 rows,
                                         qint32 cols,
                                         quint8 opacity, const QBitArray & channelFlags) const
{
    Q_UNUSED( opacity );
    Q_UNUSED( srcRowSize );

    qint32 linesize = pixelSize * sizeof(quint8) * cols;
    quint8 *d;
    const quint8 *s;

    d = dst;
    s = src;
    if ( channelFlags.isEmpty() ) {
        while (rows-- > 0) {
            memset(d, 0, linesize);
            d += dstRowSize;
        }
    }
    else {
        while ( rows-- > 0 ) {
            for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                if ( channelFlags.testBit( channel ) )
                    memset( d, 0, pixelSize );
                ++d;
            }
        }
    }
}


void KisRgbU8CompositeOp::compositeDissolve(qint32 pixelSize,
                                            quint8 *dst,
                                            qint32 dstRowSize,
                                            const quint8 *src,
                                            qint32 srcRowSize,
                                            qint32 rows,
                                            qint32 cols,
                                            quint8 opacity, const QBitArray & channelFlags) const
{
    if (opacity == OPACITY_TRANSPARENT)
        return;

    quint8 *d;
    const quint8 *s;

    qint32 i;

    double sAlpha, dAlpha;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            // XXX: correct?
            if (s[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT) continue;

            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8) (((double) sAlpha*s[RgbU8Traits::red_pos]+
                                                   (UINT8_MAX -sAlpha)*d[RgbU8Traits::red_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]= (quint8) (((double) sAlpha*s[RgbU8Traits::green_pos]+
                                                      (UINT8_MAX -sAlpha)*d[RgbU8Traits::green_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos] = (quint8) (((double) sAlpha*s[RgbU8Traits::blue_pos]+
                                                      (UINT8_MAX -sAlpha)*d[RgbU8Traits::blue_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos] = OPACITY_OPAQUE;

        }
        dst += dstRowSize;
        src += srcRowSize;

    }
}

void KisRgbU8CompositeOp::compositeCopy(qint32 pixelSize,
                                        quint8 *dstRowStart,
                                        qint32 dstRowStride,
                                        const quint8 *srcRowStart,
                                        qint32 srcRowStride,
                                        const quint8 *maskRowStart,
                                        qint32 maskRowStride,
                                        qint32 rows,
                                        qint32 numColumns,
                                        quint8 opacity, const QBitArray & channelFlags) const
{

    Q_UNUSED(maskRowStart);
    Q_UNUSED(maskRowStride);

    quint8 *dst = dstRowStart;
    const quint8 *src = srcRowStart;

    if ( channelFlags.isEmpty() ) {
        while (rows > 0) {

            memcpy(dst, src, numColumns * pixelSize);

            if (opacity != OPACITY_OPAQUE) {
                m_colorSpace->multiplyAlpha(dst, opacity, numColumns);
            }

            dst += dstRowStride;
            src += srcRowStride;
            --rows;
        }
    }
    else {
        while ( rows > 0 ) {
            for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                if ( channelFlags.testBit( channel ) ) {
                    memcpy( dst, src, pixelSize );
                }
                ++dst;
                ++src;
            }
            // XXX: how about the opacity here?
            --rows;
        }
    }
}
