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
#include <KoU8ColorSpaceTrait.h>

#include "kis_rgb_u8_compositeop.h"
#include "kis_rgb_colorspace.h"
#include "kis_color_conversions.h"

#define PixelIntensity(pixel) ((unsigned int) \
   (((double)306.0 * (pixel[PIXEL_RED]) + \
     (double)601.0 * (pixel[PIXEL_GREEN]) + \
     (double)117.0 * (pixel[PIXEL_BLUE)) \
    / 1024.0))

#define PixelIntensityToQuantum(pixel) ((quint8)PixelIntensity(pixel))

#define PixelIntensityToDouble(pixel) ((double)PixelIntensity(pixel))

#define RoundSignedToQuantum(value) ((quint8) (value < 0 ? 0 : \
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

    Q_UNUSED(channelFlags);
    
    if ( m_id == COMPOSITE_UNDEF ) {
        // Undefined == no composition
    }
    else if ( m_id == COMPOSITE_OVER ) {
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if ( m_id == COMPOSITE_IN ) {
        compositeIn(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if ( m_id == COMPOSITE_OUT ) {
        compositeOut(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if ( m_id == COMPOSITE_ATOP ) {
        compositeAtop(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if ( m_id == COMPOSITE_XOR ) {
        compositeXor(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if ( m_id == COMPOSITE_PLUS ) {
        compositePlus(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_MINUS) {
        compositeMinus(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_ADD) {
        compositeAdd(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_SUBTRACT) {
        compositeSubtract(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_DIFF) {
        compositeDiff(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_MULT) {
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_DIVIDE) {
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_BUMPMAP) {
        compositeBumpmap(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_COPY) {
        compositeCopy(m_pixelSize, dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_COPY_RED) {
        compositeCopyRed(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_COPY_GREEN) {
        compositeCopyGreen(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_COPY_BLUE) {
        compositeCopyBlue(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_COPY_OPACITY) {
        compositeCopyOpacity(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_CLEAR) {
        compositeClear(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_DISSOLVE) {
        compositeDissolve(m_pixelSize, dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_NO) {
        // No composition.
    }
    else if (m_id == COMPOSITE_DARKEN) {
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_LIGHTEN) {
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_HUE) {
        compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_SATURATION) {
        compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_VALUE) {
        compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_COLOR) {
        compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_SCREEN) {
        compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_OVERLAY) {
        compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_ERASE) {
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_DODGE) {
        compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }
    else if (m_id == COMPOSITE_BURN) {
        compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
    }

}




void KisRgbU8CompositeOp::compositeOver(quint8 *dstRowStart, qint32 dstRowStride,
                                     const quint8 *srcRowStart, qint32 srcRowStride,
                                     const quint8 *maskRowStart, qint32 maskRowStride,
                                     qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, MAX_CHANNEL_RGBA * sizeof(quint8));
                } else {
                    quint8 dstAlpha = dst[PIXEL_ALPHA];

                    quint8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, MAX_CHANNEL_RGB * sizeof(quint8));
                    } else {
                        dst[PIXEL_RED] = UINT8_BLEND(src[PIXEL_RED], dst[PIXEL_RED], srcBlend);
                        dst[PIXEL_GREEN] = UINT8_BLEND(src[PIXEL_GREEN], dst[PIXEL_GREEN], srcBlend);
                        dst[PIXEL_BLUE] = UINT8_BLEND(src[PIXEL_BLUE], dst[PIXEL_BLUE], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                quint8 srcColor = src[PIXEL_RED];
                quint8 dstColor = dst[PIXEL_RED];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_RED] = UINT8_BLEND(srcColor, dstColor, srcBlend);

                srcColor = src[PIXEL_GREEN];
                dstColor = dst[PIXEL_GREEN];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_GREEN] = UINT8_BLEND(srcColor, dstColor, srcBlend);

                srcColor = src[PIXEL_BLUE];
                dstColor = dst[PIXEL_BLUE];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_BLUE] = UINT8_BLEND(srcColor, dstColor, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin((dstColor * (UINT8_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin((dstColor * (UINT8_MAX + 1)) / (UINT8_MAX + 1 - srcColor), UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin(((UINT8_MAX - dstColor) * (UINT8_MAX + 1)) / (srcColor + 1), UINT8_MAX);
                    if (UINT8_MAX - srcColor > UINT8_MAX) srcColor = UINT8_MAX;

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin(srcColor, dstColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_RGB; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMax(srcColor, dstColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeHue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(srcHue, dstSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeSaturation(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, srcSaturation, dstValue, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeValue(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcValue;
                int dstHue;
                int dstSaturation;
                int dstValue;

                rgb_to_hsv(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcSaturation, &srcValue);
                rgb_to_hsv(dstRed, dstGreen, dstBlue, &dstHue, &dstSaturation, &dstValue);

                int srcRed;
                int srcGreen;
                int srcBlue;

                hsv_to_rgb(dstHue, dstSaturation, srcValue, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisRgbU8CompositeOp::compositeColor(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity) const
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_ALPHA];
            quint8 dstAlpha = dst[PIXEL_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                int dstRed = dst[PIXEL_RED];
                int dstGreen = dst[PIXEL_GREEN];
                int dstBlue = dst[PIXEL_BLUE];

                int srcHue;
                int srcSaturation;
                int srcLightness;
                int dstHue;
                int dstSaturation;
                int dstLightness;

                rgb_to_hls(src[PIXEL_RED], src[PIXEL_GREEN], src[PIXEL_BLUE], &srcHue, &srcLightness, &srcSaturation);
                rgb_to_hls(dstRed, dstGreen, dstBlue, &dstHue, &dstLightness, &dstSaturation);

                quint8 srcRed;
                quint8 srcGreen;
                quint8 srcBlue;

                hls_to_rgb(srcHue, dstLightness, srcSaturation, &srcRed, &srcGreen, &srcBlue);

                dst[PIXEL_RED] = UINT8_BLEND(srcRed, dstRed, srcBlend);
                dst[PIXEL_GREEN] = UINT8_BLEND(srcGreen, dstGreen, srcBlend);
                dst[PIXEL_BLUE] = UINT8_BLEND(srcBlue, dstBlue, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_RGBA;
            dst += MAX_CHANNEL_RGBA;
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
            quint8 /*opacity*/) const
{
    qint32 i;
    quint8 srcAlpha;

    while (rows-- > 0)
    {
        const quint8 *s = src;
        quint8 *d = dst;
        const quint8 *mask = srcAlphaMask;

        for (i = cols; i > 0; i--, s+=MAX_CHANNEL_RGBA, d+=MAX_CHANNEL_RGBA)
        {
            srcAlpha = s[PIXEL_ALPHA];
            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, *mask);
                mask++;
            }
            d[PIXEL_ALPHA] = UINT8_MULT(srcAlpha, d[PIXEL_ALPHA]);
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
         quint8 opacity) const
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

            if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
            {
                memcpy(d, s, pixelSize * sizeof(quint8));
                continue;
            }
            if (d[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
                continue;

            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            alpha=(double) (((double) UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) / UINT8_MAX);
            d[PIXEL_RED]=(quint8) (((double) UINT8_MAX - sAlpha) *
                        (UINT8_MAX-dAlpha) * s[PIXEL_RED] / UINT8_MAX / alpha + 0.5);
            d[PIXEL_GREEN]=(quint8) (((double) UINT8_MAX - sAlpha)*
                          (UINT8_MAX-dAlpha) * s[PIXEL_GREEN] / UINT8_MAX / alpha + 0.5);
            d[PIXEL_BLUE]=(quint8) (((double) UINT8_MAX - sAlpha)*
                         (UINT8_MAX - dAlpha) * s[PIXEL_BLUE] / UINT8_MAX / alpha + 0.5);
            d[PIXEL_ALPHA]=(quint8) ((d[PIXEL_ALPHA] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);

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
           quint8 opacity) const
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
            if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
            {
                memcpy(d, s, pixelSize * sizeof(quint8));
                break;
            }
            if (d[PIXEL_ALPHA] == OPACITY_OPAQUE)
            {
                d[PIXEL_ALPHA]=OPACITY_TRANSPARENT;
                break;
            }
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            alpha=(double) (UINT8_MAX - sAlpha) * d[PIXEL_ALPHA]/UINT8_MAX;
            d[PIXEL_RED] = (quint8) (((double) UINT8_MAX - sAlpha) * dAlpha * s[PIXEL_RED] / UINT8_MAX / alpha + 0.5);
            d[PIXEL_GREEN] = (quint8) (((double) UINT8_MAX - sAlpha) * dAlpha * s[PIXEL_GREEN] / UINT8_MAX / alpha + 0.5);
            d[PIXEL_BLUE] = (quint8) (((double) UINT8_MAX - sAlpha) * dAlpha * s[PIXEL_BLUE] / UINT8_MAX / alpha + 0.5);
            d[PIXEL_ALPHA]=(quint8) ((d[PIXEL_ALPHA] * (UINT8_MAX - alpha) / UINT8_MAX) + 0.5);
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
           quint8 opacity) const
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
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            alpha = ((double)(UINT8_MAX - sAlpha) *
                   (UINT8_MAX - dAlpha) + (double) sAlpha *
                   (UINT8_MAX - dAlpha)) / UINT8_MAX;

            red = ((double)(UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) *  s[PIXEL_RED] / UINT8_MAX +
                 (double) sAlpha * (UINT8_MAX-dAlpha) * d[PIXEL_RED]/UINT8_MAX) / alpha;
            d[PIXEL_RED] = (quint8) (red > UINT8_MAX ? UINT8_MAX : red + 0.5);

            green = ((double) (UINT8_MAX - sAlpha) * (UINT8_MAX - dAlpha) * s[PIXEL_GREEN] / UINT8_MAX +
                 (double) sAlpha * (UINT8_MAX-dAlpha) * d[PIXEL_GREEN]/UINT8_MAX)/alpha;
            d[PIXEL_GREEN] = (quint8) (green > UINT8_MAX ? UINT8_MAX : green + 0.5);

            blue = ((double) (UINT8_MAX - sAlpha) * (UINT8_MAX- dAlpha) * s[PIXEL_BLUE] / UINT8_MAX +
                     (double) sAlpha * (UINT8_MAX - dAlpha) * d[PIXEL_BLUE]/UINT8_MAX) / alpha;
            d[PIXEL_BLUE] = (quint8) (blue > UINT8_MAX ? UINT8_MAX : blue + 0.5);
            d[PIXEL_ALPHA]=(quint8) (UINT8_MAX - (alpha > UINT8_MAX ? UINT8_MAX : alpha) + 0.5);
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
          quint8 opacity) const
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
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            alpha =((double) (UINT8_MAX -sAlpha)*
                dAlpha+(double) (UINT8_MAX -dAlpha)*
                sAlpha)/UINT8_MAX ;
            red=((double) (UINT8_MAX -sAlpha)*dAlpha*
                 s[PIXEL_RED]/UINT8_MAX +(double) (UINT8_MAX -dAlpha)*
                 sAlpha*d[PIXEL_RED]/UINT8_MAX )/alpha ;
            d[PIXEL_RED]=RoundSignedToQuantum(Qt::red);
            green=((double) (UINT8_MAX -sAlpha)*dAlpha*
                   s[PIXEL_GREEN]/UINT8_MAX +(double) (UINT8_MAX -dAlpha)*
                   sAlpha*d[PIXEL_GREEN]/UINT8_MAX )/alpha ;
            d[PIXEL_GREEN]=RoundSignedToQuantum(Qt::green);
            blue=((double) (UINT8_MAX -sAlpha)*dAlpha*
                  s[PIXEL_BLUE]/UINT8_MAX +(double) (UINT8_MAX -dAlpha)*
                  sAlpha*d[PIXEL_BLUE]/UINT8_MAX )/alpha ;
            d[PIXEL_BLUE]=RoundSignedToQuantum(Qt::blue);
            d[PIXEL_ALPHA]=UINT8_MAX -RoundSignedToQuantum(alpha );
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
           quint8 opacity) const
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
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            red=((double) (UINT8_MAX -sAlpha)*s[PIXEL_RED]+(double)
                 (UINT8_MAX -dAlpha)*d[PIXEL_RED])/UINT8_MAX ;
            d[PIXEL_RED]=RoundSignedToQuantum(Qt::red);
            green=((double) (UINT8_MAX -sAlpha)*s[PIXEL_GREEN]+(double)
                   (UINT8_MAX -dAlpha)*d[PIXEL_GREEN])/UINT8_MAX ;
            d[PIXEL_GREEN]=RoundSignedToQuantum(Qt::green);
            blue=((double) (UINT8_MAX -sAlpha)*s[PIXEL_BLUE]+(double)
                  (UINT8_MAX -dAlpha)*d[PIXEL_BLUE])/UINT8_MAX ;
            d[PIXEL_BLUE]=RoundSignedToQuantum(Qt::blue);
            alpha =((double) (UINT8_MAX -sAlpha)+
                (double) (UINT8_MAX -dAlpha))/UINT8_MAX ;
            d[PIXEL_ALPHA]=UINT8_MAX -RoundSignedToQuantum(alpha );
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
            quint8 opacity) const
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
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            red=((double) (UINT8_MAX -dAlpha)*d[PIXEL_RED]-
                 (double) (UINT8_MAX -sAlpha)*s[PIXEL_RED])/UINT8_MAX ;
            d[PIXEL_RED]=RoundSignedToQuantum(Qt::red);
            green=((double) (UINT8_MAX -dAlpha)*d[PIXEL_GREEN]-
                   (double) (UINT8_MAX -sAlpha)*s[PIXEL_GREEN])/UINT8_MAX ;
            d[PIXEL_GREEN]=RoundSignedToQuantum(Qt::green);
            blue=((double) (UINT8_MAX -dAlpha)*d[PIXEL_BLUE]-
                  (double) (UINT8_MAX -sAlpha)*s[PIXEL_BLUE])/UINT8_MAX ;
            d[PIXEL_BLUE]=RoundSignedToQuantum(Qt::blue);
            alpha =((double) (UINT8_MAX -dAlpha)-
                (double) (UINT8_MAX -sAlpha))/UINT8_MAX ;
            d[PIXEL_ALPHA]=UINT8_MAX -RoundSignedToQuantum(alpha );

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
          quint8 opacity) const
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
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            red=(double) s[PIXEL_RED]+d[PIXEL_RED];
            d[PIXEL_RED]=(quint8)
                (red > UINT8_MAX  ? red-=UINT8_MAX  : red+0.5);
            green=(double) s[PIXEL_GREEN]+d[PIXEL_GREEN];
            d[PIXEL_GREEN]=(quint8)
                (green > UINT8_MAX  ? green-=UINT8_MAX  : green+0.5);
            blue=(double) s[PIXEL_BLUE]+d[PIXEL_BLUE];
            d[PIXEL_BLUE]=(quint8)
                (blue > UINT8_MAX  ? blue-=UINT8_MAX  : blue+0.5);
            d[PIXEL_ALPHA]=OPACITY_OPAQUE;
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
               quint8 opacity) const
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

            red=(double) s[PIXEL_RED]-d[PIXEL_RED];
            d[PIXEL_RED]=(quint8)
                (red < 0 ? red+=UINT8_MAX  : red+0.5);
            green=(double) s[PIXEL_GREEN]-d[PIXEL_GREEN];
            d[PIXEL_GREEN]=(quint8)
                (green < 0 ? green+=UINT8_MAX  : green+0.5);
            blue=(double) s[PIXEL_BLUE]-d[PIXEL_BLUE];
            d[PIXEL_BLUE]=(quint8)
                (blue < 0 ? blue+=UINT8_MAX  : blue+0.5);
            d[PIXEL_ALPHA]=OPACITY_OPAQUE;

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
           quint8 opacity) const
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
            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            d[PIXEL_RED]=(quint8)
                AbsoluteValue(s[PIXEL_RED]-(double) d[PIXEL_RED]);
            d[PIXEL_GREEN]=(quint8)
                AbsoluteValue(s[PIXEL_GREEN]-(double) d[PIXEL_GREEN]);
            d[PIXEL_BLUE]=(quint8)
                AbsoluteValue(s[PIXEL_BLUE]-(double) d[PIXEL_BLUE]);
            d[PIXEL_ALPHA]=UINT8_MAX - (quint8)
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
              quint8 opacity) const
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
            if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
                continue;

            // And I'm not sure whether this is correct, either.
            intensity = ((double)306.0 * s[PIXEL_RED] +
                     (double)601.0 * s[PIXEL_GREEN] +
                     (double)117.0 * s[PIXEL_BLUE]) / 1024.0;

            d[PIXEL_RED]=(quint8) (((double)
                         intensity * d[PIXEL_RED])/UINT8_MAX +0.5);
            d[PIXEL_GREEN]=(quint8) (((double)
                           intensity * d[PIXEL_GREEN])/UINT8_MAX +0.5);
            d[PIXEL_BLUE]=(quint8) (((double)
                          intensity * d[PIXEL_BLUE])/UINT8_MAX +0.5);
            d[PIXEL_ALPHA]= (quint8) (((double)
                           intensity * d[PIXEL_ALPHA])/UINT8_MAX +0.5);


        }
        dst += dstRowSize;
        src += srcRowSize;
    }

}


void KisRgbU8CompositeOp::compositeCopyChannel(quint8 pixel, quint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity) const
{
    Q_UNUSED(opacity);
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
              quint8 opacity) const
{
    compositeCopyChannel(PIXEL_RED, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity);
}

void KisRgbU8CompositeOp::compositeCopyGreen(qint32 pixelSize,
            quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 srcRowSize,
            qint32 rows,
            qint32 cols,
            quint8 opacity) const
{
    compositeCopyChannel(PIXEL_GREEN, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity);
}

void KisRgbU8CompositeOp::compositeCopyBlue(qint32 pixelSize,
               quint8 *dst,
               qint32 dstRowSize,
               const quint8 *src,
               qint32 srcRowSize,
               qint32 rows,
               qint32 cols,
               quint8 opacity) const
{
    compositeCopyChannel(PIXEL_BLUE, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity);
}


void KisRgbU8CompositeOp::compositeCopyOpacity(qint32 pixelSize,
              quint8 *dst,
              qint32 dstRowSize,
              const quint8 *src,
              qint32 srcRowSize,
              qint32 rows,
              qint32 cols,
              quint8 opacity) const
{

    // XXX: mess with intensity if there isn't an alpha channel, according to GM.
    compositeCopyChannel(PIXEL_ALPHA, pixelSize, dst, dstRowSize, src, srcRowSize, rows, cols, opacity);

}


void KisRgbU8CompositeOp::compositeClear(qint32 pixelSize,
            quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 /*srcRowSize*/,
            qint32 rows,
            qint32 cols,
            quint8 /*opacity*/) const
{

    qint32 linesize = pixelSize * sizeof(quint8) * cols;
    quint8 *d;
    const quint8 *s;

    d = dst;
    s = src;

    while (rows-- > 0) {
        memset(d, 0, linesize);
        d += dstRowSize;
    }

}


void KisRgbU8CompositeOp::compositeDissolve(qint32 pixelSize,
               quint8 *dst,
               qint32 dstRowSize,
               const quint8 *src,
               qint32 srcRowSize,
               qint32 rows,
               qint32 cols,
               quint8 opacity) const
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
            if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT) continue;

            sAlpha = UINT8_MAX - s[PIXEL_ALPHA];
            dAlpha = UINT8_MAX - d[PIXEL_ALPHA];

            d[PIXEL_RED]=(quint8) (((double) sAlpha*s[PIXEL_RED]+
                          (UINT8_MAX -sAlpha)*d[PIXEL_RED])/UINT8_MAX +0.5);
            d[PIXEL_GREEN]= (quint8) (((double) sAlpha*s[PIXEL_GREEN]+
                           (UINT8_MAX -sAlpha)*d[PIXEL_GREEN])/UINT8_MAX +0.5);
            d[PIXEL_BLUE] = (quint8) (((double) sAlpha*s[PIXEL_BLUE]+
                          (UINT8_MAX -sAlpha)*d[PIXEL_BLUE])/UINT8_MAX +0.5);
            d[PIXEL_ALPHA] = OPACITY_OPAQUE;
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
               quint8 opacity) const
{
    
    Q_UNUSED( maskRowStart );
    Q_UNUSED( maskRowStride );
    quint8 *dst = dstRowStart;
    const quint8 *src = srcRowStart;
    qint32 bytesPerPixel = pixelSize;
    
    while (rows > 0) {
        memcpy(dst, src, numColumns * bytesPerPixel);
        
        if (opacity != OPACITY_OPAQUE) {
            m_colorSpace->multiplyAlpha(dst, opacity, numColumns);
        }
        
        dst += dstRowStride;
        src += srcRowStride;
        --rows;
    }
}
