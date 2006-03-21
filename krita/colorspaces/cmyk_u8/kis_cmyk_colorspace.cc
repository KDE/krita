/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can CYANistribute it and/or modify
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
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_cmyk_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_colorspace_factory_registry.h"

#include "kis_profile.h"
#include "kis_integer_maths.h"

namespace cmyk {
    const Q_INT32 MAX_CHANNEL_CMYK = 4;
    const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}

KisCmykColorSpace::KisCmykColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p) :
    KisU8BaseColorSpace(KisID("CMYK", i18n("CMYK")), TYPE_CMYK5_8, icSigCmykData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Cyan"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::cyan));
    m_channels.push_back(new KisChannelInfo(i18n("Magenta"), 1, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::magenta));
    m_channels.push_back(new KisChannelInfo(i18n("Yellow"), 2, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::yellow));
    m_channels.push_back(new KisChannelInfo(i18n("Black"), 3, KisChannelInfo::COLOR, KisChannelInfo::UINT8, 1, Qt::black));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 4, KisChannelInfo::ALPHA, KisChannelInfo::UINT8, 1, Qt::white));

    m_alphaPos = PIXEL_CMYK_ALPHA;

    init();
}

KisCmykColorSpace::~KisCmykColorSpace()
{
}

void KisCmykColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT32 totalCyan = 0, totalMagenta = 0, totalYellow = 0, totalK = 0, totalAlpha = 0;

    while (nColors--)
    {
        Q_UINT32 alpha = (*colors)[4];
        Q_UINT32 alphaTimesWeight = alpha * *weights;

        totalCyan += (*colors)[0] * alphaTimesWeight;
        totalMagenta += (*colors)[1] * alphaTimesWeight;
        totalYellow += (*colors)[2] * alphaTimesWeight;
        totalK += (*colors)[3] * alphaTimesWeight;
        totalAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    //Q_ASSERT(newAlpha <= 255*255);
    if (totalAlpha > 255*255) totalAlpha = 255*255;

    // Divide by 255.
    dst[4] =(((totalAlpha + 0x80)>>8)+totalAlpha) >>8;

    if (totalAlpha > 0) {
        totalCyan = totalCyan / totalAlpha;
        totalMagenta = totalMagenta / totalAlpha;
        totalYellow = totalYellow / totalAlpha;
        totalK = totalK / totalAlpha;
    } // else the values are already 0 too

    Q_UINT32 dstCyan = totalCyan;
    if (dstCyan > 255) dstCyan = 255;
    dst[0] = dstCyan;

    Q_UINT32 dstMagenta = totalMagenta;
    if (dstMagenta > 255) dstMagenta = 255;
    dst[1] = dstMagenta;

    Q_UINT32 dstYellow = totalYellow;
    if (dstYellow > 255) dstYellow = 255;
    dst[2] = dstYellow;

    Q_UINT32 dstK = totalK;
    if (dstK > 255) dstK = 255;
    dst[3] = dstK;
}


void KisCmykColorSpace::convolveColors(Q_UINT8** colors, Q_INT32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    Q_INT32 totalCyan = 0, totalMagenta = 0, totalYellow = 0, totalK = 0, totalAlpha = 0;

    while (nColors--)
    {
        Q_INT32 weight = *kernelValues;

        if (weight != 0) {
            totalCyan += (*colors)[PIXEL_CYAN] * weight;
            totalMagenta += (*colors)[PIXEL_MAGENTA] * weight;
            totalYellow += (*colors)[PIXEL_YELLOW] * weight;
            totalK += (*colors)[PIXEL_BLACK] * weight;
            totalAlpha += (*colors)[PIXEL_CMYK_ALPHA] * weight;
        }
        colors++;
        kernelValues++;
    }


    if (channelFlags & KisChannelInfo::FLAG_COLOR) {
        dst[PIXEL_CYAN] = CLAMP((totalCyan / factor) + offset, 0, Q_UINT8_MAX);
        dst[PIXEL_MAGENTA] = CLAMP((totalMagenta / factor) + offset, 0, Q_UINT8_MAX);
        dst[PIXEL_YELLOW] =  CLAMP((totalYellow / factor) + offset, 0, Q_UINT8_MAX);
        dst[PIXEL_BLACK] =  CLAMP((totalK / factor) + offset, 0, Q_UINT8_MAX);
    }
    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_CMYK_ALPHA] = CLAMP((totalAlpha/ factor) + offset, 0, Q_UINT8_MAX);
    }
}


void KisCmykColorSpace::invertColor(Q_UINT8 * src, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    while (nPixels--)
    {
        src[PIXEL_CYAN] = Q_UINT8_MAX - src[PIXEL_CYAN];
        src[PIXEL_MAGENTA] = Q_UINT8_MAX - src[PIXEL_MAGENTA];
        src[PIXEL_YELLOW] = Q_UINT8_MAX - src[PIXEL_YELLOW];
        src[PIXEL_BLACK] = Q_UINT8_MAX - src[PIXEL_BLACK];
        src += psize;
    }
}

void KisCmykColorSpace::applyAdjustment(const Q_UINT8 *src, Q_UINT8 *dst, KisColorAdjustment *adj, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    Q_UINT8 * tmp = new Q_UINT8[nPixels * psize];
    Q_UINT8 * tmpPtr = tmp;
    memcpy(tmp, dst, nPixels * psize);

    KisAbstractColorSpace::applyAdjustment(src, dst, adj, nPixels);

    // Copy the alpha, which lcms doesn't do for us, grumble.

    while (nPixels--)
    {
        dst[4] = tmpPtr[4];

        tmpPtr += psize;
        dst += psize;
    }

    delete [] tmp;
}

QValueVector<KisChannelInfo *> KisCmykColorSpace::channels() const
{
    return m_channels;
}

Q_UINT32 KisCmykColorSpace::nChannels() const
{
    return cmyk::MAX_CHANNEL_CMYKA;
}

Q_UINT32 KisCmykColorSpace::nColorChannels() const
{
    return cmyk::MAX_CHANNEL_CMYK;
}

Q_UINT32 KisCmykColorSpace::pixelSize() const
{
    return cmyk::MAX_CHANNEL_CMYKA;
}

void KisCmykColorSpace::getSingleChannelPixel(Q_UINT8 *dstPixel, const Q_UINT8 *srcPixel, Q_UINT32 channelIndex)
{
    if (channelIndex < (Q_UINT32)cmyk::MAX_CHANNEL_CMYKA) {

        memset(dstPixel, 0, cmyk::MAX_CHANNEL_CMYKA * sizeof(Q_UINT8));

        if (OPACITY_TRANSPARENT != 0) {
            dstPixel[PIXEL_CMYK_ALPHA] = OPACITY_TRANSPARENT;
        }

        memcpy(dstPixel + (channelIndex * sizeof(Q_UINT8)), srcPixel + (channelIndex * sizeof(Q_UINT8)), sizeof(Q_UINT8));
    }
}

void KisCmykColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    while (rows > 0) {

        const Q_UINT8 *src = srcRowStart;
        Q_UINT8 *dst = dstRowStart;
        const Q_UINT8 *mask = maskRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {

            Q_UINT8 srcAlpha = src[PIXEL_CMYK_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(srcAlpha, U8_mask);
                }
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, cmyk::MAX_CHANNEL_CMYKA * sizeof(Q_UINT8));
                } else {
                    Q_UINT8 dstAlpha = dst[PIXEL_CMYK_ALPHA];

                    Q_UINT8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_CMYK_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, cmyk::MAX_CHANNEL_CMYK * sizeof(Q_UINT8));
                    } else {
                        dst[PIXEL_CYAN] = UINT8_BLEND(src[PIXEL_CYAN], dst[PIXEL_CYAN], srcBlend);
                        dst[PIXEL_MAGENTA] = UINT8_BLEND(src[PIXEL_MAGENTA], dst[PIXEL_MAGENTA], srcBlend);
                        dst[PIXEL_YELLOW] = UINT8_BLEND(src[PIXEL_YELLOW], dst[PIXEL_YELLOW], srcBlend);
                        dst[PIXEL_BLACK] = UINT8_BLEND(src[PIXEL_BLACK], dst[PIXEL_BLACK], srcBlend);
                    }
                }
            }

            columns--;
            src += cmyk::MAX_CHANNEL_CMYKA;
            dst += cmyk::MAX_CHANNEL_CMYKA;
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
        const Q_UINT8 *src = srcRowStart; \
        Q_UINT8 *dst = dstRowStart; \
        Q_INT32 columns = numColumns; \
        const Q_UINT8 *mask = maskRowStart; \
    \
        while (columns > 0) { \
    \
            Q_UINT8 srcAlpha = src[PIXEL_CMYK_ALPHA]; \
            Q_UINT8 dstAlpha = dst[PIXEL_CMYK_ALPHA]; \
    \
            srcAlpha = QMIN(srcAlpha, dstAlpha); \
    \
            if (mask != 0) { \
                Q_UINT8 U8_mask = *mask; \
    \
                if (U8_mask != OPACITY_OPAQUE) { \
                    srcAlpha = UINT8_MULT(srcAlpha, U8_mask); \
} \
                mask++; \
} \
    \
            if (srcAlpha != OPACITY_TRANSPARENT) { \
    \
                if (opacity != OPACITY_OPAQUE) { \
                    srcAlpha = UINT8_MULT(srcAlpha, opacity); \
} \
    \
                Q_UINT8 srcBlend; \
    \
                if (dstAlpha == OPACITY_OPAQUE) { \
                    srcBlend = srcAlpha; \
} else { \
                    Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha); \
                    dst[PIXEL_CMYK_ALPHA] = newAlpha; \
    \
                    if (newAlpha != 0) { \
                        srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha); \
} else { \
                        srcBlend = srcAlpha; \
} \
}

#define COMMON_COMPOSITE_OP_EPILOG() \
} \
    \
            columns--; \
            src += cmyk::MAX_CHANNEL_CMYKA; \
            dst += cmyk::MAX_CHANNEL_CMYKA; \
} \
    \
        rows--; \
        srcRowStart += srcRowStride; \
        dstRowStart += dstRowStride; \
        if(maskRowStart) { \
            maskRowStart += maskRowStride; \
} \
}

void KisCmykColorSpace::compositeMultiply(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {

        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {
            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = UINT8_MULT(srcColor, dstColor);

            dst[channel] = UINT8_BLEND(srcColor, dstColor, srcBlend);
        }


    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeDivide(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = QMIN((dstColor * (UINT8_MAX + 1u) + (srcColor / 2u)) / (1u + srcColor), UINT8_MAX);

            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeScreen(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = UINT8_MAX - UINT8_MULT(UINT8_MAX - dstColor, UINT8_MAX - srcColor);

            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeOverlay(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = UINT8_MULT(dstColor, dstColor + 2u * UINT8_MULT(srcColor, UINT8_MAX - dstColor));

            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeDodge(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = QMIN((dstColor * (UINT8_MAX + 1u)) / (UINT8_MAX + 1u - srcColor), UINT8_MAX);

            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeBurn(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = QMIN(((UINT8_MAX - dstColor) * (UINT8_MAX + 1u)) / (srcColor + 1u), UINT8_MAX);
            if (srcColor > UINT8_MAX - srcColor) srcColor = UINT8_MAX;
            
            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeDarken(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = QMIN(srcColor, dstColor);

            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeLighten(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    COMMON_COMPOSITE_OP_PROLOG();

    {
        for (int channel = 0; channel < cmyk::MAX_CHANNEL_CMYK; channel++) {

            Q_UINT8 srcColor = src[channel];
            Q_UINT8 dstColor = dst[channel];

            srcColor = QMAX(srcColor, dstColor);

            Q_UINT8 newColor = UINT8_BLEND(srcColor, dstColor, srcBlend);

            dst[channel] = newColor;
        }
    }

    COMMON_COMPOSITE_OP_EPILOG();
}

void KisCmykColorSpace::compositeErase(Q_UINT8 *dst,
                                          Q_INT32 dstRowSize,
                                          const Q_UINT8 *src,
                                          Q_INT32 srcRowSize,
                                          const Q_UINT8 *srcAlphaMask,
                                          Q_INT32 maskRowStride,
                                          Q_INT32 rows,
                                          Q_INT32 cols,
                                          Q_UINT8 /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const Q_UINT8 *mask = srcAlphaMask;

        for (Q_INT32 i = cols; i > 0; i--, s++, d++)
        {
            Q_UINT8 srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, U8_mask);
                }
                mask++;
            }
            d->alpha = UINT8_MULT(srcAlpha, d->alpha);
        }

        dst += dstRowSize;
        src += srcRowSize;
        if(srcAlphaMask) {
            srcAlphaMask += maskRowStride;
        }
    }
}

void KisCmykColorSpace::bitBlt(Q_UINT8 *dst,
                                  Q_INT32 dstRowStride,
                                  const Q_UINT8 *src,
                                  Q_INT32 srcRowStride,
                                  const Q_UINT8 *mask,
                                  Q_INT32 maskRowStride,
                                  Q_UINT8 opacity,
                                  Q_INT32 rows,
                                  Q_INT32 cols,
                                  const KisCompositeOp& op)
{

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
            compositeCopy(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
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

KisCompositeOpList KisCmykColorSpace::userVisiblecompositeOps() const
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
