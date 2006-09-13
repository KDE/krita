/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_ycbcr_u16_colorspace.h"

#include <qimage.h>

#include <KoIntegerMaths.h>

const Q_INT32 MAX_CHANNEL_YCbCr = 3;
const Q_INT32 MAX_CHANNEL_YCbCrA = 4;

KisYCbCrU16ColorSpace::KisYCbCrU16ColorSpace(KoColorSpaceRegistry* parent, KoColorProfile* p)
    : KoU16ColorSpaceTrait(KoID("YCbCrAU16", i18n("YCbCr (16-bit integer/channel)")), TYPE_YCbCr_16, icSigYCbCrData, parent, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Y"), "Y", PIXEL_Y * sizeof(Q_UINT16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(Q_UINT16)));
    m_channels.push_back(new KoChannelInfo(i18n("Cb"), "Cb", PIXEL_Cb * sizeof(Q_UINT16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(Q_UINT16)));
    m_channels.push_back(new KoChannelInfo(i18n("Cr"), "Cr", PIXEL_Cr * sizeof(Q_UINT16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(Q_UINT16)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), "A", PIXEL_ALPHA * sizeof(Q_UINT16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(Q_UINT16)));

    m_alphaPos = PIXEL_ALPHA * sizeof(Q_UINT16);
}


KisYCbCrU16ColorSpace::~KisYCbCrU16ColorSpace()
{
}

void KisYCbCrU16ColorSpace::setPixel(Q_UINT8 *dst, Q_UINT16 Y, Q_UINT16 Cb, Q_UINT16 Cr, Q_UINT16 alpha) const
{
    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->Y = Y;
    dstPixel->Cb = Cb;
    dstPixel->Cr = Cr;
    dstPixel->alpha = alpha;
}

void KisYCbCrU16ColorSpace::getPixel(const Q_UINT8 *src, Q_UINT16 *Y, Q_UINT16 *Cb, Q_UINT16 *Cr, Q_UINT16 *alpha) const
{
    const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);

    *Y = srcPixel->Y;
    *Cb = srcPixel->Cb;
    *Cr = srcPixel->Cr;
    *alpha = srcPixel->alpha;

}

void KisYCbCrU16ColorSpace::fromQColor(const QColor& c, Q_UINT8 *dstU8, KoColorProfile * profile )
{
    if(getProfile())
    {
        KisYCbCrU16ColorSpace::fromQColor(c, dstU8, profile);
    } else {
        Pixel *dst = reinterpret_cast<Pixel *>(dstU8);
        dst->Y = computeY( c.red(), c.green(), c.blue());
        dst->Cb = computeCb( c.red(), c.green(), c.blue());
        dst->Cr = computeCr( c.red(), c.green(), c.blue());
    }
}

void KisYCbCrU16ColorSpace::fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dstU8, KoColorProfile * profile )
{
    if(getProfile())
    {
        KisYCbCrU16ColorSpace::fromQColor(c, opacity, dstU8, profile);
    } else {
        Pixel *dst = reinterpret_cast<Pixel *>(dstU8);
        dst->Y = computeY( c.red(), c.green(), c.blue());
        dst->Cb = computeCb( c.red(), c.green(), c.blue());
        dst->Cr = computeCr( c.red(), c.green(), c.blue());
        dst->alpha = opacity;
    }
}

void KisYCbCrU16ColorSpace::toQColor(const Q_UINT8 *srcU8, QColor *c, KoColorProfile * profile)
{
    if(getProfile())
    {
        KisYCbCrU16ColorSpace::toQColor(srcU8, c, profile);
        
    } else {
        const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);
        c->setRgb(computeRed(src->Y,src->Cb,src->Cr) >> 8, computeGreen(src->Y,src->Cb,src->Cr) >> 8, computeBlue(src->Y,src->Cb,src->Cr) >> 8);
    }
}

void KisYCbCrU16ColorSpace::toQColor(const Q_UINT8 *srcU8, QColor *c, Q_UINT8 *opacity, KoColorProfile * profile)
{
    if(getProfile())
    {
        KisYCbCrU16ColorSpace::toQColor(srcU8, c, opacity, profile);
    } else {
        const Pixel *src = reinterpret_cast<const Pixel *>(srcU8);
        c->setRgb(computeRed(src->Y,src->Cb,src->Cr) >> 8, computeGreen(src->Y,src->Cb,src->Cr) >> 8, computeBlue(src->Y,src->Cb,src->Cr) >> 8);
        *opacity = src->alpha;
    }
}

Q_UINT8 KisYCbCrU16ColorSpace::difference(const Q_UINT8 *src1U8, const Q_UINT8 *src2U8)
{
    if(getProfile())
        return KisYCbCrU16ColorSpace::difference(src1U8, src2U8);
    const Pixel *src1 = reinterpret_cast<const Pixel *>(src1U8);
    const Pixel *src2 = reinterpret_cast<const Pixel *>(src2U8);

    return QMAX(QABS(src2->Y - src1->Y), QMAX(QABS(src2->Cb - src1->Cb), QABS(src2->Cr - src1->Cr))) >> 8;

}

void KisYCbCrU16ColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT16 totalY = 0, totalCb = 0, totalCr = 0, newAlpha = 0;

    while (nColors--)
    {
        const Pixel *pixel = reinterpret_cast<const Pixel *>(*colors);

        Q_UINT16 alpha = pixel->alpha;
        float alphaTimesWeight = alpha * *weights;

        totalY += (Q_UINT16)(pixel->Y * alphaTimesWeight);
        totalCb += (Q_UINT16)(pixel->Cb * alphaTimesWeight);
        totalCr += (Q_UINT16)(pixel->Cr * alphaTimesWeight);
        newAlpha += (Q_UINT16)(alphaTimesWeight);

        weights++;
        colors++;
    }

    Pixel *dstPixel = reinterpret_cast<Pixel *>(dst);

    dstPixel->alpha = newAlpha;

    if (newAlpha > 0) {
        totalY = totalY / newAlpha;
        totalCb = totalCb / newAlpha;
        totalCr = totalCr / newAlpha;
    }

    dstPixel->Y = totalY;
    dstPixel->Cb = totalCb;
    dstPixel->Cr = totalCr;
}

QValueVector<KoChannelInfo *> KisYCbCrU16ColorSpace::channels() const {
    return m_channels;
}

Q_UINT32 KisYCbCrU16ColorSpace::nChannels() const {
    return MAX_CHANNEL_YCbCrA;
}

Q_UINT32 KisYCbCrU16ColorSpace::nColorChannels() const {
    return MAX_CHANNEL_YCbCr;
}

Q_UINT32 KisYCbCrU16ColorSpace::pixelSize() const {
    return MAX_CHANNEL_YCbCrA*sizeof(Q_UINT8);
}


QImage KisYCbCrU16ColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height, KoColorProfile *  dstProfile, Q_INT32 renderingIntent, float exposure )
{
    if(getProfile())
        return KisYCbCrU16ColorSpace::convertToQImage( data, width, height, dstProfile, renderingIntent, exposure);
    
    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    img.setAlphaBuffer(true);

    Q_INT32 i = 0;
    uchar *j = img.bits();

    while ( i < width * height * MAX_CHANNEL_YCbCrA) {
        Q_UINT16 Y = *( data + i + PIXEL_Y );
        Q_UINT16 Cb = *( data + i + PIXEL_Cb );
        Q_UINT16 Cr = *( data + i + PIXEL_Cr );
#ifdef __BIG_ENDIAN__
        *( j + 0)  = *( data + i + PIXEL_ALPHA ) >> 8;
        *( j + 1 ) = computeRed(Y,Cb,Cr) >> 8;
        *( j + 2 ) = computeGreen(Y,Cb,Cr) >> 8;
        *( j + 3 ) = computeBlue(Y,Cr,Cr) >> 8;
#else
        *( j + 3)  = *( data + i + PIXEL_ALPHA ) >> 8;
        *( j + 2 ) = computeRed(Y,Cb,Cr) >> 8;
        *( j + 1 ) = computeGreen(Y,Cb,Cr) >> 8;
        *( j + 0 ) = computeBlue(Y,Cb,Cr) >> 8;
/*        *( j + 2 ) = Y;
        *( j + 1 ) = Cb;
        *( j + 0 ) = Cr;*/
#endif
        i += MAX_CHANNEL_YCbCrA;
        j += MAX_CHANNEL_YCbCrA;
    }
    return img;
}


void KisYCbCrU16ColorSpace::bitBlt(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *srcAlphaMask, Q_INT32 maskRowStride, Q_UINT8 opacity, Q_INT32 rows, Q_INT32 cols, const KoCompositeOp* op)
{
    switch (op.op()) {
        case COMPOSITE_UNDEF:
        // Undefined == no composition
            break;
        case COMPOSITE_OVER:
            compositeOver(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
            break;
        case COMPOSITE_COPY:
            compositeCopy(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
            break;
        case COMPOSITE_ERASE:
            compositeErase(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
            break;
        default:
            break;
    }
}

void KisYCbCrU16ColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *maskRowStart, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    while (rows > 0) {

        const Q_UINT16 *src = reinterpret_cast<const Q_UINT16 *>(srcRowStart);
        Q_UINT16 *dst = reinterpret_cast<Q_UINT16 *>(dstRowStart);
        const Q_UINT8 *mask = maskRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {

            Q_UINT16 srcAlpha = src[PIXEL_ALPHA];

            // apply the alphamask
            if (mask != 0) {
                Q_UINT8 U8_mask = *mask;

                if (U8_mask != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, UINT8_TO_UINT16(U8_mask));
                }
                mask++;
            }

            if (srcAlpha != U16_OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT16_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == U16_OPACITY_OPAQUE) {
                    memcpy(dst, src, MAX_CHANNEL_YCbCrA * sizeof(Q_UINT16));
                } else {
                    Q_UINT16 dstAlpha = dst[PIXEL_ALPHA];

                    Q_UINT16 srcBlend;

                    if (dstAlpha == U16_OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT16 newAlpha = dstAlpha + UINT16_MULT(U16_OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dst[PIXEL_ALPHA] = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT16_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == U16_OPACITY_OPAQUE) {
                        memcpy(dst, src, MAX_CHANNEL_YCbCr * sizeof(Q_UINT16));
                    } else {
                        dst[PIXEL_Y] = UINT16_BLEND(src[PIXEL_Y], dst[PIXEL_Y], srcBlend);
                        dst[PIXEL_Cb] = UINT16_BLEND(src[PIXEL_Cb], dst[PIXEL_Cb], srcBlend);
                        dst[PIXEL_Cr] = UINT16_BLEND(src[PIXEL_Cr], dst[PIXEL_Cr], srcBlend);
                    }
                }
            }

            columns--;
            src += MAX_CHANNEL_YCbCrA;
            dst += MAX_CHANNEL_YCbCrA;
        }

        rows--;
        srcRowStart += srcRowStride;
        dstRowStart += dstRowStride;
        if(maskRowStart) {
            maskRowStart += maskRowStride;
        }
    }
}

void KisYCbCrU16ColorSpace::compositeErase(Q_UINT8 *dst, Q_INT32 dstRowSize, const Q_UINT8 *src, Q_INT32 srcRowSize, const Q_UINT8 *srcAlphaMask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 cols, Q_UINT8 /*opacity*/)
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

KoCompositeOpList KisYCbCrU16ColorSpace::userVisiblecompositeOps() const
{
    KoCompositeOpList list;

    list.append(KoCompositeOp(COMPOSITE_OVER));
    return list;
}
