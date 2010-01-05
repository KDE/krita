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
#include <KoColorSpaceTraits.h>
#include <KoColorSpace.h>
#include "KoRgbU8CompositeOp.h"
#include "KoRgbU8ColorSpace.h"
#include "KoColorConversions.h"

#define PixelIntensity(pixel) ((unsigned int)                           \
                               (((qreal)306.0 * (pixel[KoRgbU8Traits::red_pos]) + \
                                 (qreal)601.0 * (pixel[KoRgbU8Traits::green_pos]) + \
                                 (qreal)117.0 * (pixel[KoRgbU8Traits::blue_pos)) \
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
    Q_ASSERT(channelFlags.size() == 4 || channelFlags.isEmpty());

    if (id() == COMPOSITE_UNDEF) {
        // Undefined == no composition
    } else if (id() == COMPOSITE_IN) {
        compositeIn(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_OUT) {
        compositeOut(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_DIFF) {
        compositeDiff(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_BUMPMAP) {
        compositeBumpmap(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_CLEAR) {
        compositeClear(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_DISSOLVE) {
        compositeDissolve(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_DARKEN) {
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_LIGHTEN) {
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_HUE) {
        compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_SATURATION) {
        compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_VALUE) {
        compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity, channelFlags);
    } else if (id() == COMPOSITE_COLOR) {
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

            quint8 srcAlpha = src[KoRgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[KoRgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[KoRgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos)) {
                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = qMin(srcColor, dstColor);

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += KoRgbU8Traits::channels_nb;
            dst += KoRgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart)
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

            quint8 srcAlpha = src[KoRgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[KoRgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[KoRgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                    if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos)) {
                        quint8 srcColor = src[channel];
                        quint8 dstColor = dst[channel];

                        srcColor = qMax(srcColor, dstColor);

                        quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                        dst[channel] = newColor;
                    }
                }
            }

            columns--;
            src += KoRgbU8Traits::channels_nb;
            dst += KoRgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart)
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

            quint8 srcAlpha = src[KoRgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[KoRgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[KoRgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[KoRgbU8Traits::red_pos];
                int dstGreen = dst[KoRgbU8Traits::green_pos];
                int dstBlue = dst[KoRgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[KoRgbU8Traits::red_pos], src[KoRgbU8Traits::green_pos], src[KoRgbU8Traits::blue_pos], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                    dst[KoRgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                    dst[KoRgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                    dst[KoRgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += KoRgbU8Traits::channels_nb;
            dst += KoRgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart)
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

            quint8 srcAlpha = src[KoRgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[KoRgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[KoRgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[KoRgbU8Traits::red_pos];
                int dstGreen = dst[KoRgbU8Traits::green_pos];
                int dstBlue = dst[KoRgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[KoRgbU8Traits::red_pos], src[KoRgbU8Traits::green_pos], src[KoRgbU8Traits::blue_pos], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                    dst[KoRgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                    dst[KoRgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                    dst[KoRgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += KoRgbU8Traits::channels_nb;
            dst += KoRgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart)
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

            quint8 srcAlpha = src[KoRgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[KoRgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[KoRgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[KoRgbU8Traits::red_pos];
                int dstGreen = dst[KoRgbU8Traits::green_pos];
                int dstBlue = dst[KoRgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[KoRgbU8Traits::red_pos], src[KoRgbU8Traits::green_pos], src[KoRgbU8Traits::blue_pos], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                    dst[KoRgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                    dst[KoRgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                    dst[KoRgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += KoRgbU8Traits::channels_nb;
            dst += KoRgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart)
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

            quint8 srcAlpha = src[KoRgbU8Traits::alpha_pos];
            quint8 dstAlpha = dst[KoRgbU8Traits::alpha_pos];

            srcAlpha = qMin(srcAlpha, dstAlpha);

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[KoRgbU8Traits::alpha_pos], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[KoRgbU8Traits::alpha_pos] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[KoRgbU8Traits::red_pos];
                int dstGreen = dst[KoRgbU8Traits::green_pos];
                int dstBlue = dst[KoRgbU8Traits::blue_pos];

                int srcHue;
                int srcSaturation;
                int srcLightness;
                int dstHue;
                int dstSaturation;
                int dstLightness;

                rgb_to_hls(src[KoRgbU8Traits::red_pos], src[KoRgbU8Traits::green_pos], src[KoRgbU8Traits::blue_pos], &srcHue, &srcLightness, &srcSaturation);
                rgb_to_hls(dstRed, dstGreen, dstBlue, &dstHue, &dstLightness, &dstSaturation);

                quint8 srcRed;
                quint8 srcGreen;
                quint8 srcBlue;

                hls_to_rgb(srcHue, dstLightness, srcSaturation, &srcRed, &srcGreen, &srcBlue);

                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                    dst[KoRgbU8Traits::red_pos] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                    dst[KoRgbU8Traits::green_pos] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                    dst[KoRgbU8Traits::blue_pos] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += KoRgbU8Traits::channels_nb;
            dst += KoRgbU8Traits::channels_nb;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if (maskRowStart)
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

            if (s[KoRgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT) {
                memcpy(d, s, pixelSize * sizeof(quint8));
                continue;
            }
            if (d[KoRgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
                continue;

            sAlpha = UINT8_MAX - s[KoRgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[KoRgbU8Traits::alpha_pos];

            alpha = (qreal)(((qreal) UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) / UINT8_MAX);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                d[KoRgbU8Traits::red_pos] = (quint8)(((qreal) UINT8_MAX - sAlpha) *
                                                   (UINT8_MAX - dAlpha) * s[KoRgbU8Traits::red_pos] / UINT8_MAX / alpha + 0.5);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                d[KoRgbU8Traits::green_pos] = (quint8)(((qreal) UINT8_MAX - sAlpha) *
                                                     (UINT8_MAX - dAlpha) * s[KoRgbU8Traits::green_pos] / UINT8_MAX / alpha + 0.5);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                d[KoRgbU8Traits::blue_pos] = (quint8)(((qreal) UINT8_MAX - sAlpha) *
                                                    (UINT8_MAX - dAlpha) * s[KoRgbU8Traits::blue_pos] / UINT8_MAX / alpha + 0.5);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos))
                d[KoRgbU8Traits::alpha_pos] = (quint8)((d[KoRgbU8Traits::alpha_pos] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);

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
            if (s[KoRgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT) {
                memcpy(d, s, pixelSize * sizeof(quint8));
                break;
            }
            if (d[KoRgbU8Traits::alpha_pos] == OPACITY_OPAQUE) {
                d[KoRgbU8Traits::alpha_pos] = OPACITY_TRANSPARENT;
                break;
            }
            sAlpha = UINT8_MAX - s[KoRgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[KoRgbU8Traits::alpha_pos];

            alpha = (qreal)(UINT8_MAX - sAlpha) * d[KoRgbU8Traits::alpha_pos] / UINT8_MAX;
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                d[KoRgbU8Traits::red_pos] = (quint8)(((qreal) UINT8_MAX - sAlpha) * dAlpha * s[KoRgbU8Traits::red_pos] / UINT8_MAX / alpha + 0.5);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                d[KoRgbU8Traits::green_pos] = (quint8)(((qreal) UINT8_MAX - sAlpha) * dAlpha * s[KoRgbU8Traits::green_pos] / UINT8_MAX / alpha + 0.5);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                d[KoRgbU8Traits::blue_pos] = (quint8)(((qreal) UINT8_MAX - sAlpha) * dAlpha * s[KoRgbU8Traits::blue_pos] / UINT8_MAX / alpha + 0.5);
            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos))
                d[KoRgbU8Traits::alpha_pos] = (quint8)((d[KoRgbU8Traits::alpha_pos] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);
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
            sAlpha = UINT8_MAX - s[KoRgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[KoRgbU8Traits::alpha_pos];

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                d[KoRgbU8Traits::red_pos] = (quint8)
                                          AbsoluteValue(s[KoRgbU8Traits::red_pos] - (qreal) d[KoRgbU8Traits::red_pos]);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                d[KoRgbU8Traits::green_pos] = (quint8)
                                            AbsoluteValue(s[KoRgbU8Traits::green_pos] - (qreal) d[KoRgbU8Traits::green_pos]);


            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                d[KoRgbU8Traits::blue_pos] = (quint8)
                                           AbsoluteValue(s[KoRgbU8Traits::blue_pos] - (qreal) d[KoRgbU8Traits::blue_pos]);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos))
                d[KoRgbU8Traits::alpha_pos] = UINT8_MAX - (quint8)
                                            AbsoluteValue(sAlpha - (qreal) dAlpha);

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
            if (s[KoRgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT)
                continue;

            // And I'm not sure whether this is correct, either.
            intensity = ((qreal)306.0 * s[KoRgbU8Traits::red_pos] +
                         (qreal)601.0 * s[KoRgbU8Traits::green_pos] +
                         (qreal)117.0 * s[KoRgbU8Traits::blue_pos]) / 1024.0;

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                d[KoRgbU8Traits::red_pos] = (quint8)(((qreal)
                                                    intensity * d[KoRgbU8Traits::red_pos]) / UINT8_MAX + 0.5);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                d[KoRgbU8Traits::green_pos] = (quint8)(((qreal)
                                                      intensity * d[KoRgbU8Traits::green_pos]) / UINT8_MAX + 0.5);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                d[KoRgbU8Traits::blue_pos] = (quint8)(((qreal)
                                                     intensity * d[KoRgbU8Traits::blue_pos]) / UINT8_MAX + 0.5);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos))
                d[KoRgbU8Traits::alpha_pos] = (quint8)(((qreal)
                                                      intensity * d[KoRgbU8Traits::alpha_pos]) / UINT8_MAX + 0.5);


        }
        dst += dstRowSize;
        src += srcRowSize;
    }

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
    Q_UNUSED(opacity);
    Q_UNUSED(srcRowSize);

    qint32 linesize = pixelSize * sizeof(quint8) * cols;
    quint8 *d;
    const quint8 *s;

    d = dst;
    s = src;
    if (channelFlags.isEmpty()) {
        while (rows-- > 0) {
            memset(d, 0, linesize);
            d += dstRowSize;
        }
    } else {
        while (rows-- > 0) {
            for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {
                if (channelFlags.testBit(channel))
                    memset(d, 0, pixelSize);
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
            if (s[KoRgbU8Traits::alpha_pos] == OPACITY_TRANSPARENT) continue;

            sAlpha = UINT8_MAX - s[KoRgbU8Traits::alpha_pos];
            dAlpha = UINT8_MAX - d[KoRgbU8Traits::alpha_pos];

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::red_pos))
                d[KoRgbU8Traits::red_pos] = (quint8)(((qreal) sAlpha * s[KoRgbU8Traits::red_pos] +
                                                    (UINT8_MAX - sAlpha) * d[KoRgbU8Traits::red_pos]) / UINT8_MAX + 0.5);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::green_pos))
                d[KoRgbU8Traits::green_pos] = (quint8)(((qreal) sAlpha * s[KoRgbU8Traits::green_pos] +
                                                      (UINT8_MAX - sAlpha) * d[KoRgbU8Traits::green_pos]) / UINT8_MAX + 0.5);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::blue_pos))
                d[KoRgbU8Traits::blue_pos] = (quint8)(((qreal) sAlpha * s[KoRgbU8Traits::blue_pos] +
                                                     (UINT8_MAX - sAlpha) * d[KoRgbU8Traits::blue_pos]) / UINT8_MAX + 0.5);

            if (channelFlags.isEmpty() || channelFlags.testBit(KoRgbU8Traits::alpha_pos))
                d[KoRgbU8Traits::alpha_pos] = OPACITY_OPAQUE;

        }
        dst += dstRowSize;
        src += srcRowSize;

    }
}
