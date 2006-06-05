/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <lcms.h>

#include <QImage>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include "kis_gray_u16_colorspace.h"
#include "KoU16ColorSpaceTrait.h"
#include "kis_color_conversions.h"
#include "KoIntegerMaths.h"
#include "KoColorSpaceFactoryRegistry.h"

namespace {
    const qint32 MAX_CHANNEL_GRAY = 1;
    const qint32 MAX_CHANNEL_GRAYA = 2;
}

KisGrayU16ColorSpace::KisGrayU16ColorSpace(KoColorSpaceFactoryRegistry * parent, KoColorProfile *p) :
    KoColorSpace(KoID("GRAYA16", i18n("Grayscale (16-bit integer/channel)")), parent)
    , KoU16ColorSpaceTrait(PIXEL_ALPHA * sizeof(quint16))
    , KoLcmsColorSpaceTrait(TYPE_GRAYA_16, icSigGrayData, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Gray"), PIXEL_GRAY * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), PIXEL_ALPHA * sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

/*    LPGAMMATABLE Gamma = cmsBuildGamma(256, 2.2);
    cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
    cmsFreeGamma(Gamma);
*/

    init();
}

KisGrayU16ColorSpace::~KisGrayU16ColorSpace()
{
}

void KisGrayU16ColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    quint32 totalGray = 0, newAlpha = 0;

    while (nColors--)
    {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        quint32 alpha = pixel->alpha;
        quint32 alphaTimesWeight = UINT16_MULT(alpha, UINT8_TO_UINT16(*weights));

        totalGray += UINT16_MULT(pixel->gray, alphaTimesWeight);
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= U16_OPACITY_OPAQUE);

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > 0) {
        totalGray = UINT16_DIVIDE(totalGray, newAlpha);
    }

    dstPixel->gray = totalGray;
}

void KisGrayU16ColorSpace::convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst,
                                          qint32 factor, qint32 offset, qint32 nColors) const
{
    qint32 totalGray = 0, totalAlpha = 0;

    while (nColors--)
    {
        const Pixel * pixel = reinterpret_cast<const Pixel *>( *colors );

        qint32 weight = *kernelValues;

        if (weight != 0) {
            totalGray += pixel->gray * weight;
            totalAlpha += pixel->alpha * weight;
        }
        colors++;
        kernelValues++;
    }

    Pixel * p = reinterpret_cast< Pixel *>( dst );

    if (channelFlags & KoChannelInfo::FLAG_COLOR) {
        p->gray = CLAMP( ( totalGray / factor) + offset, 0, quint16_MAX);
    }
    if (channelFlags & KoChannelInfo::FLAG_ALPHA) {
        p->alpha = CLAMP((totalAlpha/ factor) + offset, 0, quint16_MAX);
    }
}


void KisGrayU16ColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * p = reinterpret_cast< Pixel *>( src );
        p->gray = quint16_MAX - p->gray;
        src += psize;
    }
}



quint8 KisGrayU16ColorSpace::intensity8(const quint8 * src) const
{
    const Pixel * p = reinterpret_cast<const Pixel *>( src );
    return UINT16_TO_UINT8(p->gray);
}


Q3ValueVector<KoChannelInfo *> KisGrayU16ColorSpace::channels() const
{
    return m_channels;
}

quint32 KisGrayU16ColorSpace::nChannels() const
{
    return MAX_CHANNEL_GRAYA;
}

quint32 KisGrayU16ColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_GRAY;
}

quint32 KisGrayU16ColorSpace::pixelSize() const
{
    return MAX_CHANNEL_GRAYA * sizeof(quint16);
}

void KisGrayU16ColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    while (rows > 0) {

        const quint16 *src = reinterpret_cast<const quint16 *>(srcRowStart);
        quint16 *dst = reinterpret_cast<quint16 *>(dstRowStart);
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            quint16 srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                quint8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }

            if (srcAlpha != U16_OPACITY_TRANSPARENT) {

                if (opacity != U16_OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == U16_OPACITY_OPAQUE) {
                    memcpy(dst, src, MAX_CHANNEL_GRAYA * sizeof(quint16));
                } else {
                    quint16 dstAlpha = dst[PIXEL_ALPHA];

                    quint16 srcBlend;

                    if (dstAlpha == U16_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        quint16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == U16_OPACITY_OPAQUE) {
                        memcpy(dst, src, MAX_CHANNEL_GRAY * sizeof(quint16));
                    } else {
                        dst[PIXEL_GRAY] = UINT16_BLEND(src[PIXEL_GRAY], dst[PIXEL_GRAY], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_GRAYA;
            dst += MAX_CHANNEL_GRAYA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart) {
            maskRowStart += maskRowStride;
        }
    }
}

#define COMMON_COMPOSITE_OP_PROLOG() \
    while (rows > 0) { \
    \
        const quint16 *src = reinterpret_cast<const quint16 *>(srcRowStart); \
        quint16 *dst = reinterpret_cast<quint16 *>(dstRowStart); \
        qint32 columns = numColumns; \
        const quint8 *mask = maskRowStart; \
    \
        while (columns > 0) { \
    \
            quint16 srcAlpha = src[PIXEL_ALPHA]; \
            quint16 dstAlpha = dst[PIXEL_ALPHA]; \
    \
            srcAlpha = qMin(srcAlpha, dstAlpha); \
    \
            if (mask != 0) { \
                quint8 U8_mask = *mask; \
    \
                if (U8_mask != OPACITY_OPAQUE) { \
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask)); \
                } \
                mask++; \
            } \
    \
            if (srcAlpha != U16_OPACITY_TRANSPARENT) { \
    \
                if (opacity != U16_OPACITY_OPAQUE) { \
                    srcAlpha = UINT16_MULT(srcAlpha, opacity); \
                } \
    \
                quint16 srcBlend; \
    \
                if (dstAlpha == U16_OPACITY_OPAQUE) { \
                    srcBlend = srcAlpha; \
                } else { \
                    quint16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha); \
                    dst[PIXEL_ALPHA] = newAlpha; \
    \
                    if (newAlpha != 0) { \
                        srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha); \
                    } else { \
                        srcBlend = srcAlpha; \
                    } \
                }

#define COMMON_COMPOSITE_OP_EPILOG() \
            } \
    \
            columns--; \
            src += MAX_CHANNEL_GRAYA; \
            dst += MAX_CHANNEL_GRAYA; \
        } \
    \
        rows--; \
        srcRowStart += srcRowStride; \
        dstRowStart += dstRowStride; \
        if(maskRowStart) { \
            maskRowStart += maskRowStride; \
        } \
    }

void KisGrayU16ColorSpace::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        quint16 srcColor = src[PIXEL_GRAY];
        quint16 dstColor = dst[PIXEL_GRAY];

        srcColor = UINT16_MULT(srcColor, dstColor);

        dst[PIXEL_GRAY] = UINT16_BLEND(srcColor, dstColor, srcBlend);

    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin((dstColor * (UINT16_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT16_MAX);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = UINT16_MAX - UINT16_MULT(UINT16_MAX - dstColor, UINT16_MAX - srcColor);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = UINT16_MULT(dstColor, dstColor + 2u * UINT16_MULT(srcColor, UINT16_MAX - dstColor));

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin((dstColor * (UINT16_MAX + 1u)) / (UINT16_MAX + 1u - srcColor), UINT16_MAX);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin(((UINT16_MAX - dstColor) * (UINT16_MAX + 1u)) / (srcColor + 1u), UINT16_MAX);
            srcColor = qBound(0u, UINT16_MAX - srcColor, UINT16_MAX);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin(srcColor, dstColor);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisGrayU16ColorSpace::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_GRAY; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMax(srcColor, dstColor);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}


void KisGrayU16ColorSpace::compositeErase(quint8 *dst,
            qint32 dstRowSize,
            const quint8 *src,
            qint32 srcRowSize,
            const quint8 *srcAlphaMask,
            qint32 maskRowStride,
            qint32 rows,
            qint32 cols,
            quint16 /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const quint8 *mask = srcAlphaMask;

        for (qint32 i = cols; i > 0; i--, s++, d++)
        {
            quint16 srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                quint8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_BLEND(srcAlpha, U16_OPACITY_OPAQUE, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }
            d->alpha = UINT16_MULT(srcAlpha, d->alpha);
        }

        dst += dstRowSize;
        src += srcRowSize;
        if(srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisGrayU16ColorSpace::bitBlt(quint8 *dst,
                      qint32 dstRowStride,
                      const quint8 *src,
                      qint32 srcRowStride,
                      const quint8 *mask,
                      qint32 maskRowStride,
                      quint8 U8_opacity,
                      qint32 rows,
                      qint32 cols,
                      const KoCompositeOp& op)
{
    quint16 opacity = UINT8_TO_UINT16(U8_opacity);

    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_IN:
        //compositeIn(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
    case COMPOSITE_OUT:
        //compositeOut(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ATOP:
        //compositeAtop(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_XOR:
        //compositeXor(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_PLUS:
        //compositePlus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MINUS:
        //compositeMinus(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ADD:
        //compositeAdd(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SUBTRACT:
        //compositeSubtract(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIFF:
        //compositeDiff(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_MULT:
        compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIVIDE:
        compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BUMPMAP:
        //compositeBumpmap(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, U8_opacity);
        break;
    case COMPOSITE_COPY_RED:
        //compositeCopyRed(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_GREEN:
        //compositeCopyGreen(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_BLUE:
        //compositeCopyBlue(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_OPACITY:
        //compositeCopyOpacity(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_CLEAR:
        //compositeClear(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISSOLVE:
        //compositeDissolve(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DISPLACE:
        //compositeDisplace(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
#if 0
    case COMPOSITE_MODULATE:
        compositeModulate(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_THRESHOLD:
        compositeThreshold(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
#endif
    case COMPOSITE_NO:
        // No composition.
        break;
    case COMPOSITE_DARKEN:
        compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LIGHTEN:
        compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_HUE:
        //compositeHue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SATURATION:
        //compositeSaturation(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_VALUE:
        //compositeValue(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLOR:
        //compositeColor(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COLORIZE:
        //compositeColorize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LUMINIZE:
        //compositeLuminize(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_SCREEN:
        compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_OVERLAY:
        compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DODGE:
        compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BURN:
        compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    default:
        break;
    }
}

KoCompositeOpList KisGrayU16ColorSpace::userVisiblecompositeOps() const
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
