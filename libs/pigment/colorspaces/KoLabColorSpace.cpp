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
#include <lcms.h>

#include <QImage>

#include <kdebug.h>
#include <klocale.h>

#include "KoLabColorSpace.h"
#include "KoIntegerMaths.h"

KoLabColorSpace::KoLabColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p)
    : KoColorSpace("LABA", i18n("L*a*b* (16-bit integer/channel)"), parent)
    , KoU16ColorSpaceTrait(CHANNEL_ALPHA * sizeof(quint16))
    , KoLcmsColorSpaceTrait(
        COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1),
         icSigLabData, p)

{
    m_channels.push_back(new KoChannelInfo(i18n("Lightness"), CHANNEL_L * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(100,100,100)));
    m_channels.push_back(new KoChannelInfo(i18n("a*"), CHANNEL_A * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(150,150,150)));
    m_channels.push_back(new KoChannelInfo(i18n("b*"), CHANNEL_B * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(200,200,200)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), CHANNEL_ALPHA * sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    init();
}

KoLabColorSpace::~KoLabColorSpace()
{
}

void KoLabColorSpace::toLabA16(const quint8 * src, quint8 *dst, const quint32 nPixels) const
{
    memcpy( dst,  src,  nPixels * pixelSize() );
}

void KoLabColorSpace::fromLabA16(const quint8 *src, quint8 *dst, const quint32 nPixels) const
{
    memcpy( dst, src,  nPixels * pixelSize() );
}


quint8 KoLabColorSpace::difference(const quint8 *src1, const quint8 *src2)
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
        return qint8(diff);
}

void KoLabColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    quint32 totalLightness = 0, totalAlpha = 0;
    quint32 totala = 0, totalb = 0;

    while (nColors--)
    {
        const Pixel *color = reinterpret_cast<const Pixel *>( *colors );
        quint32 alphaTimesWeight = UINT8_MULT(color->alpha, *weights);

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

void KoLabColorSpace::invertColor(quint8 * src, qint32 nPixels)
{
    quint32 psize = pixelSize();

    while (nPixels--)
    {
        Pixel * s = reinterpret_cast<Pixel *>( src );

        s->lightness = MAX_CHANNEL_L - s->lightness;
        s->a = MAX_CHANNEL_AB - s->a;
        s->b = MAX_CHANNEL_AB - s->b;

        src += psize;
    }
}

void KoLabColorSpace::convolveColors(quint8** colors, qint32 * kernelValues, KoChannelInfo::enumChannelFlags channelFlags,
                                      quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    // XXX: Either do this native, or do this in 16 bit rgba, not this, which is going back to QColor!
    KoLcmsColorSpaceTrait::convolveColors(colors, kernelValues, channelFlags, dst, factor, offset, nColors);
}

void KoLabColorSpace::darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const
{
    // XXX: Is the 255 right for u16 colorspaces?
    quint32 pSize = pixelSize();
    while ( nPixels-- ) {
        const Pixel * s = reinterpret_cast<const Pixel*>( src );
        Pixel * d = reinterpret_cast<Pixel*>( dst );

        if ( compensate ) {
            d->lightness = static_cast<quint16>( ( s->lightness * shade ) / ( compensation * 255 ) );
        }
        else {
            d->lightness = static_cast<quint16>( s->lightness * shade / 255 );
        }
        d->a = s->a;
        d->b = s->b;
        d->alpha = s->alpha;

        src += pSize;
        dst += pSize;
    }
}


Q3ValueVector<KoChannelInfo *> KoLabColorSpace::channels() const
{
    return m_channels;
}

quint32 KoLabColorSpace::nChannels() const
{
    return NUM_CHANNELS;
}

quint32 KoLabColorSpace::nColorChannels() const
{
    return NUM_COLOR_CHANNELS;
}

quint32 KoLabColorSpace::pixelSize() const
{
    return sizeof(Pixel);
}

void KoLabColorSpace::getSingleChannelPixel(quint8 *dst, const quint8 *src, quint32 channelIndex)
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

void KoLabColorSpace::compositeOver(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint16 opacity)
{
    while (rows > 0) {
        const Pixel *src = reinterpret_cast<const Pixel *>(srcRowStart);
        Pixel *dst = reinterpret_cast<Pixel *>(dstRowStart);
        const quint8 *mask = maskRowStart;
        qint32 columns = numColumns;

        while (columns > 0) {

            quint16 srcAlpha = src->alpha;

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
                    quint16 dstAlpha = dst->alpha;

                    quint16 srcBlend;

                    if (dstAlpha == U16_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        quint16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha);
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

void KoLabColorSpace::compositeErase(quint8 *dst,
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

void KoLabColorSpace::bitBlt(quint8 *dst,
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

KoCompositeOpList KoLabColorSpace::userVisiblecompositeOps() const
{
    KoCompositeOpList list;

    list.append(KoCompositeOp(COMPOSITE_OVER));

    return list;
}

QString KoLabColorSpace::channelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
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

QString KoLabColorSpace::normalisedChannelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
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

