/*
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
#include <string.h>

#include <lcms.h>

#include <QImage>

#include <kdebug.h>
#include <klocale.h>

#include <kis_debug_areas.h>
#include "kis_cmyk_u16_colorspace.h"
#include "kis_u16_base_colorspace.h"
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"
#include "kis_colorspace_factory_registry.h"

namespace {
    const qint32 MAX_CHANNEL_CMYK = 4;
    const qint32 MAX_CHANNEL_CMYKA = 5;
}

KisCmykU16ColorSpace::KisCmykU16ColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisU16BaseColorSpace(KisID("CMYKA16", i18n("CMYK (16-bit integer/channel)")), TYPE_CMYK5_16, icSigCmykData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Cyan"), 0 * sizeof(quint16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    m_channels.push_back(new KisChannelInfo(i18n("Magenta"), 1 * sizeof(quint16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    m_channels.push_back(new KisChannelInfo(i18n("Yellow"), 2 * sizeof(quint16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    m_channels.push_back(new KisChannelInfo(i18n("Black"), 3 * sizeof(quint16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(quint16), Qt::black));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 4 * sizeof(quint16), KisChannelInfo::ALPHA, KisChannelInfo::UINT16, sizeof(quint16)));

    m_alphaPos = PIXEL_ALPHA * sizeof(quint16);

    init();
}

KisCmykU16ColorSpace::~KisCmykU16ColorSpace()
{
}

void KisCmykU16ColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    quint32 totalCyan = 0, totalMagenta = 0, totalYellow = 0, totalBlack = 0, newAlpha = 0;

    while (nColors--)
    {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        quint32 alpha = pixel->alpha;
        quint32 alphaTimesWeight = UINT16_MULT(alpha, UINT8_TO_UINT16(*weights));

        totalCyan += UINT16_MULT(pixel->cyan, alphaTimesWeight);
        totalMagenta += UINT16_MULT(pixel->magenta, alphaTimesWeight);
        totalYellow += UINT16_MULT(pixel->yellow, alphaTimesWeight);
        totalBlack += UINT16_MULT(pixel->black, alphaTimesWeight);
        newAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    Q_ASSERT(newAlpha <= U16_OPACITY_OPAQUE);

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > 0) {
        totalCyan = UINT16_DIVIDE(totalCyan, newAlpha);
        totalMagenta = UINT16_DIVIDE(totalMagenta, newAlpha);
        totalYellow = UINT16_DIVIDE(totalYellow, newAlpha);
        totalBlack = UINT16_DIVIDE(totalBlack, newAlpha);
    }

    dstPixel->cyan = totalCyan;
    dstPixel->magenta = totalMagenta;
    dstPixel->yellow = totalYellow;
    dstPixel->black = totalBlack;
}

void KisCmykU16ColorSpace::convolveColors(quint8** colors, qint32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    qint32 totalCyan = 0, totalMagenta = 0, totalYellow = 0, totalK = 0, totalAlpha = 0;

    while (nColors--)
    {
        const Pixel * pixel = reinterpret_cast<const Pixel *>( *colors );

        qint32 weight = *kernelValues;

        if (weight != 0) {
            totalCyan += pixel->cyan * weight;
            totalMagenta += pixel->magenta * weight;
            totalYellow += pixel->yellow * weight;
            totalK += pixel->black * weight;
            totalAlpha += pixel->alpha * weight;
        }
        colors++;
        kernelValues++;
    }

    Pixel * p = reinterpret_cast< Pixel *>( dst );

    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        p->cyan = CLAMP( ( totalCyan / factor) + offset, 0, quint16_MAX);
        p->magenta = CLAMP( ( totalMagenta / factor) + offset, 0, quint16_MAX);
        p->yellow = CLAMP( ( totalYellow / factor) + offset, 0, quint16_MAX);
        p->black = CLAMP( ( totalK / factor) + offset, 0, quint16_MAX);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        p->alpha = CLAMP((totalAlpha/ factor) + offset, 0, quint16_MAX);
    }
}


void KisCmykU16ColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * p = reinterpret_cast< Pixel *>( src );
        p->cyan = quint16_MAX - p->cyan;
        p->magenta = quint16_MAX - p->magenta;
        p->yellow = quint16_MAX - p->yellow;
        p->black = quint16_MAX - p->black;
        src += psize;
    }
}



void KisCmykU16ColorSpace::applyAdjustment(const quint8 *src, quint8 *dst, KisColorAdjustment *adj, qint32 nPixels)
{
    quint32 psize = pixelSize();
    
    quint8 * tmp = new quint8[nPixels * psize];
    quint8 * tmpPtr = tmp;
    memcpy(tmp, dst, nPixels * psize);
    
    KisAbstractColorSpace::applyAdjustment(src, dst, adj, nPixels);

    // Copy the alpha, which lcms doesn't do for us, grumble.

    while (nPixels--)
    {
        quint16 *pixelAlphaSrc = reinterpret_cast<quint16 *>(tmpPtr + m_alphaPos);
        quint16 *pixelAlphaDst = reinterpret_cast<quint16 *>(dst + m_alphaPos);
        
        *pixelAlphaDst= *pixelAlphaSrc;
        
        tmpPtr += psize;
        dst += psize;
    }

    delete [] tmp;
}

Q3ValueVector<KisChannelInfo *> KisCmykU16ColorSpace::channels() const
{
    return m_channels;
}

quint32 KisCmykU16ColorSpace::nChannels() const
{
    return MAX_CHANNEL_CMYKA;
}

quint32 KisCmykU16ColorSpace::nColorChannels() const
{
    return MAX_CHANNEL_CMYK;
}

quint32 KisCmykU16ColorSpace::pixelSize() const
{
    return MAX_CHANNEL_CMYKA * sizeof(quint16);
}

void KisCmykU16ColorSpace::getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex)
{
    if (channelIndex < (quint32)MAX_CHANNEL_CMYKA) {

        memset(dstPixel, 0, MAX_CHANNEL_CMYKA * sizeof(quint16));

        if (U16_OPACITY_TRANSPARENT != 0) {
            dstPixel[PIXEL_ALPHA] = U16_OPACITY_TRANSPARENT;
        }

        memcpy(dstPixel + (channelIndex * sizeof(quint16)), srcPixel + (channelIndex * sizeof(quint16)), sizeof(quint16));
    }
}

void KisCmykU16ColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
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
                    memcpy(dst, src, MAX_CHANNEL_CMYKA * sizeof(quint16));
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
                        memcpy(dst, src, MAX_CHANNEL_CMYK * sizeof(quint16));
                    } else {
                        dst[PIXEL_CYAN] = UINT16_BLEND(src[PIXEL_CYAN], dst[PIXEL_CYAN], srcBlend);
                        dst[PIXEL_MAGENTA] = UINT16_BLEND(src[PIXEL_MAGENTA], dst[PIXEL_MAGENTA], srcBlend);
                        dst[PIXEL_YELLOW] = UINT16_BLEND(src[PIXEL_YELLOW], dst[PIXEL_YELLOW], srcBlend);
                        dst[PIXEL_BLACK] = UINT16_BLEND(src[PIXEL_BLACK], dst[PIXEL_BLACK], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_CMYKA;
            dst += MAX_CHANNEL_CMYKA;
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
            src += MAX_CHANNEL_CMYKA; \
            dst += MAX_CHANNEL_CMYKA; \
        } \
    \
        rows--; \
        srcRowStart += srcRowStride; \
        dstRowStart += dstRowStride; \
        if(maskRowStart) { \
            maskRowStart += maskRowStride; \
        } \
    }

void KisCmykU16ColorSpace::compositeMultiply(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {

        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {
            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = UINT16_MULT(srcColor, dstColor);

            dst[channel] = UINT16_BLEND(srcColor, dstColor, srcBlend);
        }


    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeDivide(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin((dstColor * (UINT16_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT16_MAX);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeScreen(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = UINT16_MAX - UINT16_MULT(UINT16_MAX - dstColor, UINT16_MAX - srcColor);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeOverlay(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = UINT16_MULT(dstColor, dstColor + 2u * UINT16_MULT(srcColor, UINT16_MAX - dstColor));

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeDodge(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin((dstColor * (UINT16_MAX + 1u)) / (UINT16_MAX + 1u - srcColor), UINT16_MAX);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeBurn(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin(((UINT16_MAX - dstColor) * (UINT16_MAX + 1u)) / (srcColor + 1u), UINT16_MAX);
            if (srcColor > UINT16_MAX - srcColor) srcColor = UINT16_MAX;
            
            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeDarken(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMin(srcColor, dstColor);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeLighten(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < MAX_CHANNEL_CMYK; channel++) {

            quint16 srcColor = src[channel];
            quint16 dstColor = dst[channel];

            srcColor = qMax(srcColor, dstColor);

            quint16 newColor = UINT16_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykU16ColorSpace::compositeErase(quint8 *dst,
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

void KisCmykU16ColorSpace::bitBlt(quint8 *dst,
                      qint32 dstRowStride,
                      const quint8 *src,
                      qint32 srcRowStride,
                      const quint8 *mask,
                      qint32 maskRowStride,
                      quint8 U8_opacity,
                      qint32 rows,
                      qint32 cols,
                      const KisCompositeOp& op)
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
    case COMPOSITE_COPY_CYAN:
        //compositeCopyCyan(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_MAGENTA:
        //compositeCopyMagenta(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY_YELLOW:
        //compositeCopyYellow(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
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

KisCompositeOpList KisCmykU16ColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));
    list.append(KisCompositeOp(COMPOSITE_MULT));
    list.append(KisCompositeOp(COMPOSITE_BURN));
    list.append(KisCompositeOp(COMPOSITE_DODGE));
    list.append(KisCompositeOp(COMPOSITE_DIVIDE));
    list.append(KisCompositeOp(COMPOSITE_SCREEN));
    list.append(KisCompositeOp(COMPOSITE_OVERLAY));
    list.append(KisCompositeOp(COMPOSITE_DARKEN));
    list.append(KisCompositeOp(COMPOSITE_LIGHTEN));

    return list;
}
