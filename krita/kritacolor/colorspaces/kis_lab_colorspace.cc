/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_lab_colorspace.h"
#include "kis_u16_base_colorspace.h"
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"

KisLabColorSpace::KisLabColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p)
    : KisAbstractColorSpace(KisID("LABA", i18n("L*a*b* (16-bit integer/channel)")),
        COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1),
         icSigLabData, parent, p)

{
    m_channels.push_back(new KisChannelInfo(i18n("Lightness"), 0, KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(100,100,100)));
    m_channels.push_back(new KisChannelInfo(i18n("a*"), 2, KisChannelInfo::COLOR, KisChannelInfo::INT16, sizeof(Q_INT16), QColor(150,150,150)));
    m_channels.push_back(new KisChannelInfo(i18n("b*"), 4, KisChannelInfo::COLOR, KisChannelInfo::INT16, sizeof(Q_INT16), QColor(200,200,200)));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 6, KisChannelInfo::ALPHA, KisChannelInfo::UINT16, sizeof(Q_UINT16)));

    init();
}

KisLabColorSpace::~KisLabColorSpace()
{
}

Q_UINT8 KisLabColorSpace::difference(const Q_UINT8 *src1, const Q_UINT8 *src2)
{
    cmsCIELab labF1, labF2;

    if (getAlpha(src1) == OPACITY_TRANSPARENT || getAlpha(src2) == OPACITY_TRANSPARENT)
        return (getAlpha(src1) == getAlpha(src2) ? 0 : 255);

    cmsLabEncoded2Float(&labF1, (WORD *)src1);
    cmsLabEncoded2Float(&labF2, (WORD *)src2);
    double diff = cmsDeltaE(&labF1, &labF2);
    if(diff>255)
        return 255;
    else
        return Q_INT8(diff);
}

void KisLabColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT32 totalLightness = 0, totalAlpha = 0;
    Q_INT32 totala = 0, totalb = 0;

    while (nColors--)
    {
        const Pixel *color = reinterpret_cast<const Pixel *>( *colors );
        Q_UINT32 alphaTimesWeight = UINT8_MULT(color->alpha, *weights);

        totalLightness += color->lightness * alphaTimesWeight;
        totala += color->a * alphaTimesWeight;
        totalb += color->b * alphaTimesWeight;
        totalAlpha += alphaTimesWeight;

        weights++;
        colors++;
    }

    if (totalAlpha > UINT16_MAX)
        totalAlpha = UINT16_MAX;
    ((Pixel *)dst)->alpha = totalAlpha;

    if (totalAlpha > 0) {
        totalLightness /= totalAlpha;
        totala /= totalAlpha;
        totalb /= totalAlpha;
    } // else the values are already 0 too

    if (totalLightness > UINT16_MAX)
        totalLightness = UINT16_MAX;
    ((Pixel *)dst)->lightness = totalLightness;

    if (totala > INT16_MAX)
        totala = INT16_MAX;
    ((Pixel *)dst)->a = totala;

    if (totalb > INT16_MAX)
        totalb = INT16_MAX;
    ((Pixel *)dst)->b = totalb;
}

void KisLabColorSpace::invertColor(Q_UINT8 * src, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * s = reinterpret_cast<Pixel *>( src );

        s->lightness = Q_UINT16_MAX - s->lightness;
        s->a = Q_UINT16_MAX - s->a;
        s->b = Q_UINT16_MAX - s->b;

        src += psize;
    }
}

void KisLabColorSpace::convolveColors(Q_UINT8** colors, Q_INT32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags,
                                      Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    // XXX: Either do this native, or do this in 16 bit rgba, not this, which is going back to QColor!
    KisAbstractColorSpace::convolveColors(colors, kernelValues, channelFlags, dst, factor, offset, nColors);
}

void KisLabColorSpace::darken(const Q_UINT8 * src, Q_UINT8 * dst, Q_INT32 shade, bool compensate, double compensation, Q_INT32 nPixels) const
{
    // XXX: Is the 255 right for u16 colorspaces?
    Q_UINT32 pSize = pixelSize();
    while ( nPixels-- ) {
        const Pixel * s = reinterpret_cast<const Pixel*>( src );
        Pixel * d = reinterpret_cast<Pixel*>( dst );

        if ( compensate ) {
            d->lightness = static_cast<Q_UINT16>( ( s->lightness * shade ) / ( compensation * 255 ) );
        }
        else {
            d->lightness = static_cast<Q_UINT16>( s->lightness * shade / 255 );
        }
        d->a = s->a;
        d->b = s->b;
        d->alpha = s->alpha;

        src += pSize;
        dst += pSize;
    }
}


QValueVector<KisChannelInfo *> KisLabColorSpace::channels() const
{
    return m_channels;
}

Q_UINT32 KisLabColorSpace::nChannels() const
{
    return 4;
}

Q_UINT32 KisLabColorSpace::nColorChannels() const
{
    return 3;
}

Q_UINT32 KisLabColorSpace::pixelSize() const
{
    return sizeof(Pixel);
}


void KisLabColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT16 opacity)
{
    while (rows > 0) {
        const Pixel *src = reinterpret_cast<const Pixel *>(srcRowStart);
        Pixel *dst = reinterpret_cast<Pixel *>(dstRowStart);
        const Q_UINT8 *mask = maskRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {

            Q_UINT16 srcAlpha = src->alpha;

            // apply the alphamask
            if (mask != 0) {
                if (*mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, *mask);
                }
                mask++;
            }

            if (srcAlpha != U16_OPACITY_TRANSPARENT) {

                if (opacity != U16_OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == U16_OPACITY_OPAQUE) {
                    memcpy(dst, src, sizeof(Pixel));
                } else {
                    Q_UINT16 dstAlpha = dst->alpha;

                    Q_UINT16 srcBlend;

                    if (dstAlpha == U16_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst->alpha = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == U16_OPACITY_OPAQUE) {
                        memcpy(dst, src, sizeof(Pixel));
                    } else {
/*printf("blend is %d\n", srcBlend);
printf("%d %d %d\n", src->lightness, src->a, src->b);
printf("%d %d %d\n", dst->lightness, dst->a, dst->b);
*/
                        dst->lightness = UINT16_BLEND(src->lightness, dst->lightness, srcBlend);
                        dst->a = INT16_BLEND(src->a, dst->a, srcBlend);
                        dst->b = INT16_BLEND(src->b, dst->b, srcBlend);
//printf("%d %d %d\n", dst->lightness, dst->a, dst->b);
                    }
                }
            }

            columns--;
            src++;
            dst++;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart) {
            maskRowStart += maskRowStride;
        }
    }
}

void KisLabColorSpace::compositeErase(Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src,
            Q_INT32 srcRowSize,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_INT32 rows,
            Q_INT32 cols,
            Q_UINT16 /*opacity*/)
{
    while (rows-- > 0)
    {
        const Pixel *s = reinterpret_cast<const Pixel *>(src);
        Pixel *d = reinterpret_cast<Pixel *>(dst);
        const Q_UINT8 *mask = srcAlphaMask;

        for (Q_INT32 i = cols; i > 0; i--, s++, d++)
        {
            Q_UINT16 srcAlpha = s->alpha;

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

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

void KisLabColorSpace::compositeCopy(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 */*maskRowStart*/, Q_INT32 /*maskRowStride*/, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    Q_UINT8 *dst = dstRowStart;
    const Q_UINT8 *src = srcRowStart;
    Q_INT32 bytesPerPixel = pixelSize();

    while (rows > 0) {
        memcpy(dst, src, numColumns * bytesPerPixel);

        if (opacity != OPACITY_OPAQUE) {
            multiplyAlpha(dst, opacity, numColumns);
        }

        dst += dstRowStride;
        src += srcRowStride;
        --rows;
    }
}

void KisLabColorSpace::bitBlt(Q_UINT8 *dst,
                      Q_INT32 dstRowStride,
                      const Q_UINT8 *src,
                      Q_INT32 srcRowStride,
                      const Q_UINT8 *mask,
                      Q_INT32 maskRowStride,
                      Q_UINT8 U8_opacity,
                      Q_INT32 rows,
                      Q_INT32 cols,
                      const KisCompositeOp& op)
{
    Q_UINT16 opacity = UINT8_TO_UINT16(U8_opacity);

    switch (op.op()) {
    case COMPOSITE_UNDEF:
        // Undefined == no composition
        break;
    case COMPOSITE_OVER:
        compositeOver(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_IN:
        //compositeIn(pixelSize(), dst, dstRowStride, src, srcRowStride, rows, cols, opacity);
        break;
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
        //compositeMultiply(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DIVIDE:
        //compositeDivide(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
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
        //compositeDarken(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_LIGHTEN:
        //compositeLighten(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
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
        //compositeScreen(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_OVERLAY:
        //compositeOverlay(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_DODGE:
        //compositeDodge(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_BURN:
        //compositeBurn(dst, dstRowStride, src, srcRowStride, mask, maskRowStride, rows, cols, opacity);
        break;
    default:
        break;
    }
}

KisCompositeOpList KisLabColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));

    return list;
}
//////////////
Q_UINT8 KisLabColorSpace::getAlpha(const Q_UINT8 * U8_pixel) const
{
    const Pixel *pix = reinterpret_cast<const Pixel *>(U8_pixel);

    return UINT16_TO_UINT8(pix->alpha);
}


void KisLabColorSpace::setAlpha(Q_UINT8 *U8_pixels, Q_UINT8 alpha, Q_INT32 nPixels) const
{
    Pixel *pix = reinterpret_cast<Pixel *>(U8_pixels);

    while (nPixels--) {
        pix->alpha = UINT8_TO_UINT16(alpha);

        ++pix;
    }
}

void KisLabColorSpace::multiplyAlpha(Q_UINT8 *U8_pixels, Q_UINT8 U8_alpha, Q_INT32 nPixels)
{
    Pixel *pix = reinterpret_cast<Pixel *>(U8_pixels);

    while (nPixels--) {
        pix->alpha = UINT8_MULT(pix->alpha, U8_alpha);

        ++pix;
    }
}

void KisLabColorSpace::applyAlphaU8Mask(Q_UINT8 * U8_pixels, Q_UINT8 * alpha8, Q_INT32 nPixels)
{
    Pixel *pix = reinterpret_cast<Pixel *>(U8_pixels);

    while (nPixels--) {
        pix->alpha = UINT8_MULT(pix->alpha, *alpha8);

        ++pix;
        ++alpha8;
    }
}

void KisLabColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * U8_pixels, Q_UINT8 * alpha8, Q_INT32 nPixels)
{
    Pixel *pix = reinterpret_cast<Pixel *>(U8_pixels);

    while(nPixels--) {
            pix->alpha = UINT8_MULT(pix->alpha, (MAX_SELECTED - *alpha8));

            ++pix;
            ++alpha8;
    }
}

QString KisLabColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    const Pixel *pix = reinterpret_cast<const Pixel *>(U8_pixel);
    Q_ASSERT(channelIndex < nChannels());
    switch(channelIndex)
    {
        case 0:
            return QString().setNum(pix->lightness);
        case 1:
            return QString().setNum(pix->a);
        case 2:
            return QString().setNum(pix->b);
        case 3:
            return QString().setNum(pix->alpha);
        default:
            return QString("Error");
    }
}

QString KisLabColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    const Pixel *pix = reinterpret_cast<const Pixel *>(U8_pixel);
    Q_ASSERT(channelIndex < nChannels());
    switch(channelIndex)
    {
        case 0:
            return QString().setNum(static_cast<float>(pix->lightness) / UINT16_MAX);
        case 1:
            return QString().setNum(static_cast<float>(pix->a) / INT16_MAX);
        case 2:
            return QString().setNum(static_cast<float>(pix->b) / INT16_MAX);
        case 3:
            return QString().setNum(static_cast<float>(pix->alpha) / UINT16_MAX);
        default:
            return QString("Error");
    }
}

Q_UINT8 KisLabColorSpace::scaleToU8(const Q_UINT8 * U8_pixel, Q_INT32 channelPos)
{
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    return UINT16_TO_UINT8(pixel[channelPos]);
}

Q_UINT16 KisLabColorSpace::scaleToU16(const Q_UINT8 * U8_pixel, Q_INT32 channelPos)
{
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    return pixel[channelPos];
}

