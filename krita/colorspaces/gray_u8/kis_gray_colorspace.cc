/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <limits.h>
#include <stdlib.h>

#include <config.h>
#include <lcms.h>

#include <QImage>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

#include "KoLcmsColorSpaceTrait.h"
#include "KoU8ColorSpaceTrait.h"
#include "kis_gray_colorspace.h"

#include "KoIntegerMaths.h"

namespace {
    const qint32 MAX_CHANNEL_GRAYSCALE = 1;
    const qint32 MAX_CHANNEL_GRAYSCALEA = 2;
}

KisGrayColorSpace::KisGrayColorSpace(KoColorSpaceFactoryRegistry * parent, KoColorProfile *p) :
    KoColorSpace(KoID("GRAYA", i18n("Grayscale")), parent)
    , KoU8ColorSpaceTrait(PIXEL_GRAY_ALPHA)
    , KoLcmsColorSpaceTrait(TYPE_GRAYA_8, icSigGrayData, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Gray"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), 1, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    init();
}


KisGrayColorSpace::~KisGrayColorSpace()
{
}

void KisGrayColorSpace::setPixel(quint8 *pixel, quint8 gray, quint8 alpha) const
{
    pixel[PIXEL_GRAY] = gray;
    pixel[PIXEL_GRAY_ALPHA] = alpha;
}

void KisGrayColorSpace::getPixel(const quint8 *pixel, quint8 *gray, quint8 *alpha) const
{
    *gray = pixel[PIXEL_GRAY];
    *alpha = pixel[PIXEL_GRAY_ALPHA];
}

void KisGrayColorSpace::getAlpha(const quint8 *pixel, quint8 *alpha) const
{
    *alpha = pixel[PIXEL_GRAY_ALPHA];
}

void KisGrayColorSpace::setAlpha(quint8 *pixels, quint8 alpha, qint32 nPixels) const
{
    while (nPixels > 0) {
        pixels[PIXEL_GRAY_ALPHA] = alpha;
        --nPixels;
        pixels += MAX_CHANNEL_GRAYSCALEA;
    }
}

void KisGrayColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    quint32 totalGray = 0, newAlpha = 0;

    while (nColors--)
    {
        quint32 alpha = (*colors)[PIXEL_GRAY_ALPHA];
        quint32 alphaTimesWeight = UINT8_MULT(alpha, *weights);

        totalGray += (*colors)[PIXEL_GRAY] * alphaTimesWeight;
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= 255);

    dst[PIXEL_GRAY_ALPHA] = newAlpha;

    if (newAlpha > 0) {
        totalGray = UINT8_DIVIDE(totalGray, newAlpha);
    }

    // Divide by 255.
    totalGray += 0x80;
    quint32 dstGray = ((totalGray >> 8) + totalGray) >> 8;
    Q_ASSERT(dstGray <= 255);
    dst[PIXEL_GRAY] = dstGray;
}

void KisGrayColorSpace::convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    qint32 totalGray = 0, totalAlpha = 0;

    while (nColors--)
    {
        qint32 weight = *kernelValues;

        if (weight != 0) {
            totalGray += (*colors)[PIXEL_GRAY] * weight;
            totalAlpha += (*colors)[PIXEL_GRAY_ALPHA] * weight;
        }
        colors++;
        kernelValues++;
    }


    if (channelFlags & KoChannelInfo::FLAG_COLOR) {
        dst[PIXEL_GRAY] = CLAMP((totalGray / factor) + offset, 0, quint8_MAX);
    }
    if (channelFlags & KoChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_GRAY_ALPHA] = CLAMP((totalAlpha/ factor) + offset, 0, quint8_MAX);
    }

}


void KisGrayColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        src[PIXEL_GRAY] = quint8_MAX - src[PIXEL_GRAY];
        src += psize;
    }
}

void KisGrayColorSpace::darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
{
    quint32 pSize = pixelSize();

    while (nPixels--) {
        if (compensate) {
            dst[PIXEL_GRAY]  = (qint8) qMin(255, (int)((((src[PIXEL_GRAY] * shade) / (compensation * 255)))));
        }
        else {
            dst[PIXEL_GRAY]  = (qint8) qMin(255, (src[PIXEL_GRAY] * shade / 255));
        }
        dst += pSize;
        src += pSize;
    }
}

quint8 KisGrayColorSpace::intensity8(const quint8 * src) const
{
    return src[PIXEL_GRAY];
}

Q3ValueVector<KoChannelInfo *> KisGrayColorSpace::channels() const
{
    return m_channels;
}

quint32 KisGrayColorSpace::nChannels() const
{
    return MAX_CHANNEL_GRAYSCALEA;
}

quint32 KisGrayColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_GRAYSCALE;
}

quint32 KisGrayColorSpace::pixelSize() const
{
    return MAX_CHANNEL_GRAYSCALEA;
}

void KisGrayColorSpace::bitBlt(quint8 *dst,
                      qint32 dstRowStride,
                      const quint8 *src,
                      qint32 srcRowStride,
                      const quint8 *mask,
                      qint32 maskRowStride,
                      quint8 opacity,
                      qint32 rows,
                      qint32 cols,
                      const KoCompositeOp& op)
{
    switch (op.op()) {
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MULT:
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIVIDE:
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DARKEN:
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LIGHTEN:
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SCREEN:
        compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_OVERLAY:
        compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DODGE:
        compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BURN:
        compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_CLEAR: {
        quint8 *d;
        qint32 linesize;

        linesize = MAX_CHANNEL_GRAYSCALEA*sizeof(quint8) * cols;
        d = dst;
        while (rows-- > 0) {
            memset(d, 0, linesize);
            d += dstRowStride;
        }
    }
        break;
    default:
        break;
    }
}

KoCompositeOpList KisGrayColorSpace::userVisiblecompositeOps() const
{
    KoCompositeOpList list;

    list.append(KoCompositeOp(COMPOSITE_OVER));
    list.append(KoCompositeOp(COMPOSITE_MULT));
    list.append(KoCompositeOp(COMPOSITE_BURN));
    list.append(KoCompositeOp(COMPOSITE_DODGE));
    list.append(KoCompositeOp(COMPOSITE_DIVIDE));
    list.append(KoCompositeOp(COMPOSITE_SCREEN));
    list.append(KoCompositeOp(COMPOSITE_OVERLAY));
    list.append(KoCompositeOp(COMPOSITE_DARKEN));
    list.append(KoCompositeOp(COMPOSITE_LIGHTEN));

    return list;
}

void KisGrayColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, MAX_CHANNEL_GRAYSCALEA * sizeof(quint8));
                } else {
                    quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

                    quint8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_GRAY_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, MAX_CHANNEL_GRAYSCALE * sizeof(quint8));
                    } else {
                        dst[PIXEL_GRAY] = UINT8_BLEND(src[PIXEL_GRAY], dst[PIXEL_GRAY], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                quint8 srcColor = src[PIXEL_GRAY];
                quint8 dstColor = dst[PIXEL_GRAY];

                srcColor = UINT8_MULT(srcColor, dstColor);

                dst[PIXEL_GRAY] = UINT8_BLEND(srcColor, dstColor, srcBlend);
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin((dstColor * (UINT8_MAX + 1)) / (1 + srcColor), UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = UINT8_MULT(dstColor, dstColor + UINT8_MULT(2 * srcColor, UINT8_MAX - dstColor));

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin((dstColor * (UINT8_MAX + 1)) / (UINT8_MAX + 1 - srcColor), UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin(((UINT8_MAX - dstColor) * (UINT8_MAX + 1)) / (srcColor + 1), UINT8_MAX);
                    srcColor = qBound(0u, UINT8_MAX - srcColor, UINT8_MAX);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMin(srcColor, dstColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity)
{
    while (rows > 0) {

        const quint8 *src = srcRowStart;
        quint8 *dst = dstRowStart;
        qint32 columns = numColumns;
        const quint8 *mask = maskRowStart;

        while (columns > 0) {

            quint8 srcAlpha = src[PIXEL_GRAY_ALPHA];
            quint8 dstAlpha = dst[PIXEL_GRAY_ALPHA];

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
                    srcAlpha = UINT8_MULT(src[PIXEL_GRAY_ALPHA], opacity);
                }

                quint8 srcBlend;

                if (dstAlpha == OPACITY_OPAQUE) {
                    srcBlend = srcAlpha;
                } else {
                    quint8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                    dst[PIXEL_GRAY_ALPHA] = newAlpha;

                    if (newAlpha != 0) {
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                    } else {
                        srcBlend = srcAlpha;
                    }
                }

                for (int channel = 0; channel < MAX_CHANNEL_GRAYSCALE; channel++) {

                    quint8 srcColor = src[channel];
                    quint8 dstColor = dst[channel];

                    srcColor = qMax(srcColor, dstColor);

                    quint8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

                    dst[channel] = newColor;
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYSCALEA;
            dst += MAX_CHANNEL_GRAYSCALEA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart)
            maskRowStart += maskRowStride;
    }
}

void KisGrayColorSpace::compositeErase(quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 srcRowSize,
            const quint8 *srcAlphaMask,
            qint32 maskRowStride,
            qint32 rows,
            qint32 cols,
            quint8 /*opacity*/)
{
    qint32 i;
    quint8 srcAlpha;

    while (rows-- > 0)
    {
        const quint8 *s = src;
        quint8 *d = dst;
        const quint8 *mask = srcAlphaMask;

        for (i = cols; i > 0; i--, s+=MAX_CHANNEL_GRAYSCALEA, d+=MAX_CHANNEL_GRAYSCALEA)
        {
            srcAlpha = s[PIXEL_GRAY_ALPHA];
            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, *mask);
                mask++;
            }
            d[PIXEL_GRAY_ALPHA] = UINT8_MULT(srcAlpha, d[PIXEL_GRAY_ALPHA]);
        }

        dst += dstRowSize;
        if(srcAlphaMask)
            srcAlphaMask += maskRowStride;
        src += srcRowSize;
    }
}
