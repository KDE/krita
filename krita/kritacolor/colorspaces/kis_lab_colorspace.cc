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
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"

KisLabColorSpace::KisLabColorSpace(KisColorSpaceFactoryRegistry * parent, KisProfile *p)
    : KisU16BaseColorSpace(KisID("LABA", i18n("L*a*b* (16-bit integer/channel)")),
        COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1),
         icSigLabData, parent, p)

{
    m_channels.push_back(new KisChannelInfo(i18n("Lightness"), i18n("L"), CHANNEL_L * sizeof(Q_UINT16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(100,100,100)));
    m_channels.push_back(new KisChannelInfo(i18n("a*"), i18n("a"), CHANNEL_A * sizeof(Q_UINT16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(150,150,150)));
    m_channels.push_back(new KisChannelInfo(i18n("b*"), i18n("b"), CHANNEL_B * sizeof(Q_UINT16), KisChannelInfo::COLOR, KisChannelInfo::UINT16, sizeof(Q_UINT16), QColor(200,200,200)));
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), i18n("A"), CHANNEL_ALPHA * sizeof(Q_UINT16), KisChannelInfo::ALPHA, KisChannelInfo::UINT16, sizeof(Q_UINT16)));

    m_alphaPos = CHANNEL_ALPHA * sizeof(Q_UINT16);

    init();
}

KisLabColorSpace::~KisLabColorSpace()
{
}

Q_UINT8 * KisLabColorSpace::toLabA16(const Q_UINT8 * data, const Q_UINT32 nPixels) const
{
    Q_UINT8 * pixels = new Q_UINT8[nPixels * pixelSize()];
    memcpy( pixels,  data,  nPixels * pixelSize() );
    return pixels;
}

Q_UINT8 * KisLabColorSpace::fromLabA16(const Q_UINT8 * labData, const Q_UINT32 nPixels) const
{
    Q_UINT8 * pixels = new Q_UINT8[nPixels * pixelSize()];
    memcpy( pixels, labData,  nPixels * pixelSize() );
    return pixels;
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
    Q_UINT32 totala = 0, totalb = 0;

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

    if (totalAlpha > UINT16_MAX) {
        totalAlpha = UINT16_MAX;
    }

    ((Pixel *)dst)->alpha = totalAlpha;

    if (totalAlpha > 0) {
        totalLightness /= totalAlpha;
        totala /= totalAlpha;
        totalb /= totalAlpha;
    } // else the values are already 0 too

    if (totalLightness > MAX_CHANNEL_L) {
        totalLightness = MAX_CHANNEL_L;
    }

    ((Pixel *)dst)->lightness = totalLightness;

    if (totala > MAX_CHANNEL_AB) {
        totala = MAX_CHANNEL_AB;
    }

    ((Pixel *)dst)->a = totala;

    if (totalb > MAX_CHANNEL_AB) {
        totalb = MAX_CHANNEL_AB;
    }

    ((Pixel *)dst)->b = totalb;
}

void KisLabColorSpace::invertColor(Q_UINT8 * src, Q_INT32 nPixels)
{
    Q_UINT32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * s = reinterpret_cast<Pixel *>( src );

        s->lightness = MAX_CHANNEL_L - s->lightness;
        s->a = MAX_CHANNEL_AB - s->a;
        s->b = MAX_CHANNEL_AB - s->b;

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
    return NUM_CHANNELS;
}

Q_UINT32 KisLabColorSpace::nColorChannels() const
{
    return NUM_COLOR_CHANNELS;
}

Q_UINT32 KisLabColorSpace::pixelSize() const
{
    return sizeof(Pixel);
}

void KisLabColorSpace::getSingleChannelPixel(Q_UINT8 *dst, const Q_UINT8 *src, Q_UINT32 channelIndex)
{
    if (channelIndex < NUM_CHANNELS) {

        const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);
        Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

        switch (channelIndex) {
        case CHANNEL_L:
            dstPixel->lightness = srcPixel->lightness;
            dstPixel->a = CHANNEL_AB_ZERO_OFFSET;
            dstPixel->b = CHANNEL_AB_ZERO_OFFSET;
            dstPixel->alpha = U16_OPACITY_TRANSPARENT;
            break;
        case CHANNEL_A:
            dstPixel->lightness = MAX_CHANNEL_L / 2;
            dstPixel->a = srcPixel->a;
            dstPixel->b = CHANNEL_AB_ZERO_OFFSET;
            dstPixel->alpha = U16_OPACITY_TRANSPARENT;
            break;
        case CHANNEL_B:
            dstPixel->lightness = MAX_CHANNEL_L / 2;
            dstPixel->a = CHANNEL_AB_ZERO_OFFSET;
            dstPixel->b = srcPixel->b;
            dstPixel->alpha = U16_OPACITY_TRANSPARENT;
            break;
        case CHANNEL_ALPHA:
            dstPixel->lightness = MAX_CHANNEL_L / 2;
            dstPixel->a = CHANNEL_AB_ZERO_OFFSET;
            dstPixel->b = CHANNEL_AB_ZERO_OFFSET;
            dstPixel->alpha = srcPixel->alpha;
            break;
        }
    }
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
                        dst->a = UINT16_BLEND(src->a, dst->a, srcBlend);
                        dst->b = UINT16_BLEND(src->b, dst->b, srcBlend);
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

QString KisLabColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    const Pixel *pix = reinterpret_cast<const Pixel *>(U8_pixel);
    Q_ASSERT(channelIndex < nChannels());
    switch(channelIndex)
    {
        case CHANNEL_L:
            return QString().setNum(pix->lightness);
        case CHANNEL_A:
            return QString().setNum(pix->a);
        case CHANNEL_B:
            return QString().setNum(pix->b);
        case CHANNEL_ALPHA:
            return QString().setNum(pix->alpha);
        default:
            return QString("Error");
    }
}

QString KisLabColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    const Pixel *pix = reinterpret_cast<const Pixel *>(U8_pixel);
    Q_ASSERT(channelIndex < nChannels());

    // These convert from lcms encoded format to standard ranges.

    switch(channelIndex)
    {
        case CHANNEL_L:
            return QString().setNum(100.0 * static_cast<float>(pix->lightness) / MAX_CHANNEL_L);
        case CHANNEL_A:
            return QString().setNum(100.0 * ((static_cast<float>(pix->a) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case CHANNEL_B:
            return QString().setNum(100.0 * ((static_cast<float>(pix->b) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
        case CHANNEL_ALPHA:
            return QString().setNum(100.0 * static_cast<float>(pix->alpha) / UINT16_MAX);
        default:
            return QString("Error");
    }
}

