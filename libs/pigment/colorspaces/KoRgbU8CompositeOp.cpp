/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *
 */

#include <KoColorSpace.h>
#include <KoIntegerMaths.h>

#include "KoRgbU8CompositeOp.h"
#include "KoRgbU8ColorSpace.h"
#include "KoColorConversions.h"

#define PixelIntensity(pixel) ((unsigned int)                           \
                               (((qreal)306.0 * (pixel[RgbU8Traits::red_pos]) + \
                                 (qreal)601.0 * (pixel[RgbU8Traits::green_pos]) + \
                                 (qreal)117.0 * (pixel[RgbU8Traits::blue_pos)) \
                                 / 1024.0))

#define PixelIntensityToQuantum(pixel) ((quint8)PixelIntensity(pixel))

#define PixelIntensityToDouble(pixel) ((qreal)PixelIntensity(pixel))

#define RoundSignedToQuantum(value) ((quint8) (value < 0 ? 0 :          \
                                               (value > UINT8_MAX) ? UINT8_MAX : value + 0.5))

#define RoundToQuantum(value) ((quint8) (value > UINT8_MAX ? UINT8_MAX : \
                                         value + 0.5))

// And from studio.h
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))


KoRgbU8CompositeOp::KoRgbU8CompositeOp(KoColorSpace * cs, const QString& id, const QString& description, const bool userVisible)
    : KoCompositeOp(cs, id, description, "", userVisible)
{
    m_pixelSize = cs->pixelSize();
}


void KoRgbU8CompositeOp::composite(quint8 *dst, qint32 dstRowStride,
                                    const quint8 *src, qint32 srcRowStride,
                                    const quint8 *mask, qint32 maskRowStride,
                                    qint32 rows, qint32 cols,
                                    quint8 opacity,
                                    const QBitArray & channelFlags) const
{
    Q_ASSERT( channelFlags.size() == 4 || channelFlags.isEmpty() );

    if ( id() == COMPOSITE_UNDEF ) {
        // Undefined == no composition
    }
    else if ( id() == COMPOSITE_IN ) {
        compositeIn(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( id() == COMPOSITE_OUT ) {
        compositeOut(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( id() == COMPOSITE_ATOP ) {
        compositeAtop(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( id() == COMPOSITE_XOR ) {
        compositeXor(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if ( id() == COMPOSITE_PLUS ) {
        compositePlus(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_MINUS) {
        compositeMinus(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_DIFF) {
        compositeDiff(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_BUMPMAP) {
        compositeBumpmap(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_COPY) {
        compositeCopy(m_pixelSize, dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_COPY_RED) {
        compositeCopyRed(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_COPY_GREEN) {
        compositeCopyGreen(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_COPY_BLUE) {
        compositeCopyBlue(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_COPY_OPACITY) {
        compositeCopyOpacity(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_CLEAR) {
        compositeClear(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_DISSOLVE) {
        compositeDissolve(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_NO) {
        // No composition.
    }
    else if (id() == COMPOSITE_DARKEN) {
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_LIGHTEN) {
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_HUE) {
        compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_SATURATION) {
        compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_VALUE) {
        compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
    else if (id() == COMPOSITE_COLOR) {
        compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    }
}


void KoRgbU8CompositeOp::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
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

void KoRgbU8CompositeOp::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
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

void KoRgbU8CompositeOp::compositeHue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
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

void KoRgbU8CompositeOp::compositeSaturation(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
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

void KoRgbU8CompositeOp::compositeValue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
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

void KoRgbU8CompositeOp::compositeColor(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity, const QBitArray & channelFlags) const
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


void KoRgbU8CompositeOp::compositeIn(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal alpha;

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

            alpha=(qreal) (((qreal) UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) / UINT8_MAX);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8) (((qreal) UINT8_MAX - sAlpha) *
                                                  (UINT8_MAX-dAlpha) * s[RgbU8Traits::red_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]=(quint8) (((qreal) UINT8_MAX - sAlpha)*
                                                    (UINT8_MAX-dAlpha) * s[RgbU8Traits::green_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos]=(quint8) (((qreal) UINT8_MAX - sAlpha)*
                                                   (UINT8_MAX - dAlpha) * s[RgbU8Traits::blue_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=(quint8) ((d[RgbU8Traits::alpha_pos] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);

        }
        dst += dstRowSize;
        src += srcRowSize;
    }
}

void KoRgbU8CompositeOp::compositeOut(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal alpha;

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

            alpha=(qreal) (UINT8_MAX - sAlpha) * d[RgbU8Traits::alpha_pos]/UINT8_MAX;
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos] = (quint8) (((qreal) UINT8_MAX - sAlpha) * dAlpha * s[RgbU8Traits::red_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos] = (quint8) (((qreal) UINT8_MAX - sAlpha) * dAlpha * s[RgbU8Traits::green_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos] = (quint8) (((qreal) UINT8_MAX - sAlpha) * dAlpha * s[RgbU8Traits::blue_pos] / UINT8_MAX / alpha + 0.5);
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=(quint8) ((d[RgbU8Traits::alpha_pos] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}

void KoRgbU8CompositeOp::compositeAtop(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            alpha = ((qreal)(UINT8_MAX - sAlpha) *
                     (UINT8_MAX - dAlpha) + (qreal) sAlpha *
                     (UINT8_MAX - dAlpha)) / UINT8_MAX;

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red = ((qreal)(UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) *  s[RgbU8Traits::red_pos] / UINT8_MAX +
                       (qreal) sAlpha * (UINT8_MAX-dAlpha) * d[RgbU8Traits::red_pos]/UINT8_MAX) / alpha;
                d[RgbU8Traits::red_pos] = (quint8) (red > UINT8_MAX ? UINT8_MAX : red + 0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green = ((qreal) (UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) * s[RgbU8Traits::green_pos] / UINT8_MAX +
                         (qreal) sAlpha * (UINT8_MAX-dAlpha) * d[RgbU8Traits::green_pos]/UINT8_MAX)/alpha;
                d[RgbU8Traits::green_pos] = (quint8) (green > UINT8_MAX ? UINT8_MAX : green + 0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue = ((qreal) (UINT8_MAX - sAlpha) * (UINT8_MAX- dAlpha) * s[RgbU8Traits::blue_pos] / UINT8_MAX +
                        (qreal) sAlpha * (UINT8_MAX - dAlpha) * d[RgbU8Traits::blue_pos]/UINT8_MAX) / alpha;
                d[RgbU8Traits::blue_pos] = (quint8) (blue > UINT8_MAX ? UINT8_MAX : blue + 0.5);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=(quint8) (UINT8_MAX - (alpha > UINT8_MAX ? UINT8_MAX : alpha) + 0.5);
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}


void KoRgbU8CompositeOp::compositeXor(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            alpha =((qreal) (UINT8_MAX -sAlpha)*
                    dAlpha+(qreal) (UINT8_MAX -dAlpha)*
                    sAlpha)/UINT8_MAX ;
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=((qreal) (UINT8_MAX -sAlpha)*dAlpha*
                     s[RgbU8Traits::red_pos]/UINT8_MAX +(qreal) (UINT8_MAX -dAlpha)*
                     sAlpha*d[RgbU8Traits::red_pos]/UINT8_MAX )/alpha ;
                d[RgbU8Traits::red_pos]=RoundSignedToQuantum(Qt::red);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=((qreal) (UINT8_MAX -sAlpha)*dAlpha*
                       s[RgbU8Traits::green_pos]/UINT8_MAX +(qreal) (UINT8_MAX -dAlpha)*
                       sAlpha*d[RgbU8Traits::green_pos]/UINT8_MAX )/alpha ;
                d[RgbU8Traits::green_pos]=RoundSignedToQuantum(Qt::green);
            }
            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=((qreal) (UINT8_MAX -sAlpha)*dAlpha*
                      s[RgbU8Traits::blue_pos]/UINT8_MAX +(qreal) (UINT8_MAX -dAlpha)*
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


void KoRgbU8CompositeOp::compositePlus(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=((qreal) (UINT8_MAX -sAlpha)*s[RgbU8Traits::red_pos]+(qreal)
                     (UINT8_MAX -dAlpha)*d[RgbU8Traits::red_pos])/UINT8_MAX ;
                d[RgbU8Traits::red_pos]=RoundSignedToQuantum(Qt::red);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=((qreal) (UINT8_MAX -sAlpha)*s[RgbU8Traits::green_pos]+(qreal)
                       (UINT8_MAX -dAlpha)*d[RgbU8Traits::green_pos])/UINT8_MAX ;
                d[RgbU8Traits::green_pos]=RoundSignedToQuantum(Qt::green);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=((qreal) (UINT8_MAX -sAlpha)*s[RgbU8Traits::blue_pos]+(qreal)
                      (UINT8_MAX -dAlpha)*d[RgbU8Traits::blue_pos])/UINT8_MAX ;
                d[RgbU8Traits::blue_pos]=RoundSignedToQuantum(Qt::blue);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                alpha =((qreal) (UINT8_MAX -sAlpha)+
                       (qreal) (UINT8_MAX -dAlpha))/UINT8_MAX ;
                d[RgbU8Traits::alpha_pos]=UINT8_MAX -RoundSignedToQuantum(alpha );
            }
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}



void KoRgbU8CompositeOp::compositeMinus(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal alpha, red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=((qreal) (UINT8_MAX -dAlpha)*d[RgbU8Traits::red_pos]-
                     (qreal) (UINT8_MAX -sAlpha)*s[RgbU8Traits::red_pos])/UINT8_MAX ;
                d[RgbU8Traits::red_pos]=RoundSignedToQuantum(Qt::red);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=((qreal) (UINT8_MAX -dAlpha)*d[RgbU8Traits::green_pos]-
                       (qreal) (UINT8_MAX -sAlpha)*s[RgbU8Traits::green_pos])/UINT8_MAX ;
                d[RgbU8Traits::green_pos]=RoundSignedToQuantum(Qt::green);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=((qreal) (UINT8_MAX -dAlpha)*d[RgbU8Traits::blue_pos]-
                      (qreal) (UINT8_MAX -sAlpha)*s[RgbU8Traits::blue_pos])/UINT8_MAX ;
                d[RgbU8Traits::blue_pos]=RoundSignedToQuantum(Qt::blue);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) ) {
                alpha =((qreal) (UINT8_MAX -dAlpha)-
                        (qreal) (UINT8_MAX -sAlpha))/UINT8_MAX ;
                d[RgbU8Traits::alpha_pos]=UINT8_MAX -RoundSignedToQuantum(alpha );
            }
        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}

void KoRgbU8CompositeOp::compositeAdd(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;
    qreal red, green, blue;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) ) {
                red=(qreal) s[RgbU8Traits::red_pos]+d[RgbU8Traits::red_pos];
                d[RgbU8Traits::red_pos]=(quint8)
                                        (red > UINT8_MAX  ? red-=UINT8_MAX  : red+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) ) {
                green=(qreal) s[RgbU8Traits::green_pos]+d[RgbU8Traits::green_pos];
                d[RgbU8Traits::green_pos]=(quint8)
                                          (green > UINT8_MAX  ? green-=UINT8_MAX  : green+0.5);
            }

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) ) {
                blue=(qreal) s[RgbU8Traits::blue_pos]+d[RgbU8Traits::blue_pos];
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


void KoRgbU8CompositeOp::compositeDiff(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8)
                                        AbsoluteValue(s[RgbU8Traits::red_pos]-(qreal) d[RgbU8Traits::red_pos]);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]=(quint8)
                                          AbsoluteValue(s[RgbU8Traits::green_pos]-(qreal) d[RgbU8Traits::green_pos]);


            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos]=(quint8)
                                         AbsoluteValue(s[RgbU8Traits::blue_pos]-(qreal) d[RgbU8Traits::blue_pos]);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]=UINT8_MAX - (quint8)
                                          AbsoluteValue(sAlpha-(qreal) dAlpha);

        }
        dst += dstRowSize;
        src += srcRowSize;

    }
}

void KoRgbU8CompositeOp::compositeBumpmap(qint32 pixelSize,
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

    qreal intensity;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            // Is this correct? It's not this way in GM.
            if (s[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
                continue;

            // And I'm not sure whether this is correct, either.
            intensity = ((qreal)306.0 * s[RgbU8Traits::red_pos] +
                         (qreal)601.0 * s[RgbU8Traits::green_pos] +
                         (qreal)117.0 * s[RgbU8Traits::blue_pos]) / 1024.0;

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8) (((qreal)
                                                   intensity * d[RgbU8Traits::red_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]=(quint8) (((qreal)
                                                     intensity * d[RgbU8Traits::green_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos]=(quint8) (((qreal)
                                                    intensity * d[RgbU8Traits::blue_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos]= (quint8) (((qreal)
                                                      intensity * d[RgbU8Traits::alpha_pos])/UINT8_MAX +0.5);


        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}


void KoRgbU8CompositeOp::compositeCopyChannel(quint8 pixel, quint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity, const QBitArray & channelFlags) const
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

void KoRgbU8CompositeOp::compositeCopyRed(qint32 pixelSize,
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

void KoRgbU8CompositeOp::compositeCopyGreen(qint32 pixelSize,
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

void KoRgbU8CompositeOp::compositeCopyBlue(qint32 pixelSize,
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


void KoRgbU8CompositeOp::compositeCopyOpacity(qint32 pixelSize,
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


void KoRgbU8CompositeOp::compositeClear(qint32 pixelSize,
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


void KoRgbU8CompositeOp::compositeDissolve(qint32 pixelSize,
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

    qreal sAlpha, dAlpha;

    while (rows-- > 0) {
        d = dst;
        s = src;
        for (i = cols; i > 0; i--, d += pixelSize, s += pixelSize) {
            // XXX: correct?
            if (s[RgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT) continue;

            sAlpha = UINT8_MAX - s[RgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[RgbU8Traits::alpha_pos];

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::red_pos ) )
                d[RgbU8Traits::red_pos]=(quint8) (((qreal) sAlpha*s[RgbU8Traits::red_pos]+
                                                   (UINT8_MAX -sAlpha)*d[RgbU8Traits::red_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::green_pos ) )
                d[RgbU8Traits::green_pos]= (quint8) (((qreal) sAlpha*s[RgbU8Traits::green_pos]+
                                                      (UINT8_MAX -sAlpha)*d[RgbU8Traits::green_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::blue_pos ) )
                d[RgbU8Traits::blue_pos] = (quint8) (((qreal) sAlpha*s[RgbU8Traits::blue_pos]+
                                                      (UINT8_MAX -sAlpha)*d[RgbU8Traits::blue_pos])/UINT8_MAX +0.5);

            if ( channelFlags.isEmpty() || channelFlags.testBit( RgbU8Traits::alpha_pos ) )
                d[RgbU8Traits::alpha_pos] = OPACITY_OPAQUE;

        }
        dst += dstRowSize;
        src += srcRowSize;

    }
}

void KoRgbU8CompositeOp::compositeCopy(qint32 pixelSize,
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
                colorSpace()->multiplyAlpha(dst, opacity, numColumns);
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
