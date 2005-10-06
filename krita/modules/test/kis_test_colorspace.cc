/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_image.h"
#include "kis_test_colorspace.h"
#include "kis_color_conversions.h"
#include "kis_integer_maths.h"

struct testcspixel
{
    Q_UINT16 bmg;
    Q_UINT8 g;
    Q_UINT8 r;
    Q_UINT8 alpha;
    Q_UINT8 dummy;
};

KisTestColorSpace::KisTestColorSpace() :
    KisAbstractColorSpace(KisID("test", i18n("TestCS/Alpha")), 0, icMaxEnumData)
{

    m_channels.push_back(new KisChannelInfo(i18n("blue+green"), 0, COLOR, sizeof(Q_UINT16)));
    m_channels.push_back(new KisChannelInfo(i18n("green"), 1, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("red"), 2, COLOR));
    m_channels.push_back(new KisChannelInfo(i18n("alpha"), 3, ALPHA));

    m_alphaPos = 3;
}

KisTestColorSpace::~KisTestColorSpace()
{
}

void KisTestColorSpace::fromQColor(const QColor& c, Q_UINT8 *dst, KisProfile *  /*profile*/)
{
    testcspixel *pix = (testcspixel *)dst;

    pix->r = c.red();
    pix->g = c.green();
    pix->bmg = c.blue()*16 + c.green();
}

void KisTestColorSpace::fromQColor(const QColor& c, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile *  /*profile*/)
{
    testcspixel *pix = (testcspixel *)dst;

    pix->r = c.red();
    pix->g = c.green();
    pix->bmg = c.blue()*16 + c.green();
    pix->alpha = opacity;
}

void KisTestColorSpace::getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha)
{
    const testcspixel *pix = (const testcspixel *)pixel;
    *alpha = pix -> alpha;
}

void KisTestColorSpace::toQColor(const Q_UINT8 *src, QColor *c, KisProfile *  /*profile*/)
{
    testcspixel *pix = (testcspixel *)src;
    c -> setRgb(pix->r, pix->g, (pix->bmg - pix->g)/16);
}

void KisTestColorSpace::toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile *  /*profile*/)
{
    testcspixel *pix = (testcspixel *)src;
    c -> setRgb(pix->r, pix->g, (pix->bmg - pix->g)/16);
    *opacity = pix->alpha;
}

Q_INT8 KisTestColorSpace::difference(const Q_UINT8 *src1, const Q_UINT8 *src2)
{
    testcspixel *pix1 = (testcspixel *)src1;
    testcspixel *pix2 = (testcspixel *)src2;
    return QMAX(QABS(pix2->r - pix1->r), QMAX(QABS(pix2->g - pix1->g),
    QABS((pix2->bmg - pix2->g - pix1->bmg + pix1->g)/16)));
}

void KisTestColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    Q_UINT32 red=0, green=0, blue=0;

    while(nColors--)
    {
        testcspixel *pix = (testcspixel *)colors;
        red += pix->r * *weights;
        green += pix->g * *weights;
        blue += pix->bmg * *weights;
        weights++;
        colors++;
    }

    // Now downscale to 8 bit
    red += 0x80;
    *dst++ = ((red >> 8) + red) >> 8;
    green += 0x80;
    *dst++ = ((green >> 8) + green) >> 8;
    blue += 0x80;
    *dst++ = ((blue >> 8) + blue) >> 8;
}

QValueVector<KisChannelInfo *> KisTestColorSpace::channels() const
{
    return m_channels;
}

bool KisTestColorSpace::hasAlpha() const
{
    return true;
}

Q_INT32 KisTestColorSpace::nChannels() const
{
    return 4;
}

Q_INT32 KisTestColorSpace::nColorChannels() const
{
    return 3;
}

Q_INT32 KisTestColorSpace::pixelSize() const
{
    return sizeof(struct testcspixel);
}

QImage KisTestColorSpace::convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
                         KisProfile *  srcProfile, KisProfile *  dstProfile,
                         Q_INT32 renderingIntent, float /*exposure*/)

{
    testcspixel *pix = (testcspixel *)data;

#ifdef __BIG_ENDIAN__
    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    img.setAlphaBuffer(true);
    // Find a way to use convertPixelsTo without needing to code a
    // complete agrb color strategy or something like that.

    Q_INT32 i = 0;
    uchar *j = img.bits();

    while ( i < width * height) {

        // Swap the bytes
        *( j + 0)  = pix->alpha/256;
        *( j + 1 ) = pix->r;
        *( j + 2 ) = pix->g;;
        *( j + 3 ) = (pix->bmg );//- pix->g)/16;

        pix++;
        i++;

        j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
    }

#else
    QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
    img.setAlphaBuffer(true);

    Q_INT32 i = 0;
    uchar *j = img.bits();

    while ( i < width * height) {

        // Swap the bytes
        *( j + 3) = pix->alpha;
        *( j + 2) = pix->r;
        *( j + 1) = pix->g;
        *( j + 0) = (pix->bmg - pix->g)/16;

        pix++;
        i++;

        j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
    }

#endif

    if (srcProfile != 0 && dstProfile != 0) {
        convertPixelsTo(img.bits(), srcProfile,
                img.bits(), this, dstProfile,
                width * height, renderingIntent);
    }

    return img;
}

void KisTestColorSpace::compositeOver(Q_UINT8 *dstRowStart, Q_INT32 dstRowStride, const Q_UINT8 *srcRowStart, Q_INT32 srcRowStride, const Q_UINT8 *srcAlphaMask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 numColumns, Q_UINT8 opacity)
{
    while (rows > 0) {

        const Q_UINT8 *src = srcRowStart;
        const Q_UINT8 *mask = srcAlphaMask;
        Q_UINT8 *dst = dstRowStart;
        Q_INT32 columns = numColumns;

        while (columns > 0) {
            testcspixel *pix = (testcspixel *)src;
            testcspixel *dstpix = (testcspixel *)dst;
            Q_UINT16 srcAlpha = pix->alpha;

            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_MULT(srcAlpha, *mask);
                mask++;
            }

            if (srcAlpha != OPACITY_TRANSPARENT) {

                if (opacity != OPACITY_OPAQUE) {
                    srcAlpha = UINT8_MULT(srcAlpha, opacity);
                }

                if (srcAlpha == OPACITY_OPAQUE) {
                    memcpy(dst, src, sizeof(struct testcspixel));
                } else {
                    Q_UINT8 dstAlpha = dstpix->alpha;

                    Q_UINT8 srcBlend;

                    if (dstAlpha == OPACITY_OPAQUE) {
                        srcBlend = srcAlpha;
                    } else {
                        Q_UINT8 newAlpha = dstAlpha + UINT8_MULT(OPACITY_OPAQUE - dstAlpha, srcAlpha);
                        dstpix->alpha = newAlpha;

                        if (newAlpha != 0) {
                            srcBlend = UINT8_DIVIDE(srcAlpha, newAlpha);
                        } else {
                            srcBlend = srcAlpha;
                        }
                    }

                    if (srcBlend == OPACITY_OPAQUE) {
                        memcpy(dst, src, sizeof(struct testcspixel));
                    } else {
                        dstpix->r = UINT8_BLEND(pix->r, dstpix->r, srcBlend);
                        dstpix->bmg = UINT8_BLEND((pix->bmg - pix->g)/16, (dstpix->bmg -dstpix->g)/16, srcBlend);
                        dstpix->g = UINT8_BLEND(pix->g, dstpix->g, srcBlend);
                        dstpix->bmg = dstpix->bmg*16 + dstpix->g;
                    }
                }
            }

            columns--;
            src += sizeof(struct testcspixel);
            dst += sizeof(struct testcspixel);
        }

        rows--;
        srcRowStart += srcRowStride;
        if(srcAlphaMask)
            srcAlphaMask += maskRowStride;
        dstRowStart += dstRowStride;
    }
}

void KisTestColorSpace::compositeErase(Q_UINT8 *dst,
            Q_INT32 dstRowSize,
            const Q_UINT8 *src,
            Q_INT32 srcRowSize,
            const Q_UINT8 *srcAlphaMask,
            Q_INT32 maskRowStride,
            Q_INT32 rows,
            Q_INT32 cols,
            Q_UINT8 /*opacity*/)
{
    Q_INT32 i;
    Q_UINT8 srcAlpha;

    while (rows-- > 0)
    {
        testcspixel *pix = (testcspixel *)src;
        testcspixel *dstpix = (testcspixel *)dst;
        const Q_UINT8 *mask = srcAlphaMask;

        for (i = cols; i > 0; i--, pix++, dstpix++)
        {
            srcAlpha = pix->alpha;
            // apply the alphamask
            if(mask != 0)
            {
                if(*mask != OPACITY_OPAQUE)
                    srcAlpha = UINT8_BLEND(srcAlpha, OPACITY_OPAQUE, *mask);
                mask++;
            }
            dstpix->alpha = UINT8_MULT(srcAlpha, dstpix->alpha);
        }

        dst += dstRowSize;
        if(srcAlphaMask)
            srcAlphaMask += maskRowStride;
        src += srcRowSize;
    }
}

void KisTestColorSpace::compositeCopy(Q_UINT8 *dst,
           Q_INT32 dstRowStride,
           const Q_UINT8 *src,
           Q_INT32 srcRowStride,
           const Q_UINT8 *srcAlphaMask,
           Q_INT32 maskRowStride,
           Q_INT32 rows,
           Q_INT32 cols,
           Q_UINT8 /*opacity*/)
{
    Q_INT32 linesize = pixelSize() * cols;
    Q_UINT8 *d;
    const Q_UINT8 *s, *mask;
    d = dst;
    s = src;
    mask = srcAlphaMask;

    if(srcAlphaMask==0)
    {
        while (rows-- > 0) {
            memcpy(d, s, linesize);
            d += dstRowStride;
            s += srcRowStride;
        }
    }
    else
    {
        while (rows-- > 0) {
            testcspixel *pix = (testcspixel *)src;
            testcspixel *dstpix = (testcspixel *)dst;
            mask = srcAlphaMask;

            for (Q_INT32 i = cols; i > 0; i--, pix++, dstpix++, mask++)
            {
                if(*mask != OPACITY_OPAQUE)
                {
                    dstpix->r = UINT8_BLEND(pix->r, dstpix->r, *mask);
                    dstpix->bmg = UINT8_BLEND((pix->bmg - pix->g)/16, (dstpix->bmg -dstpix->g)/16,     *mask);
                    dstpix->g = UINT8_BLEND(pix->g, dstpix->g, *mask);
                    dstpix->bmg = dstpix->bmg*16 + dstpix->g;
                    dstpix->alpha = UINT8_BLEND(pix->alpha, dstpix->alpha, *mask);
                }
            }
            dst += dstRowStride;
            srcAlphaMask += maskRowStride;
            src += srcRowStride;
        }
    }
}

void KisTestColorSpace::bitBlt(Q_UINT8 *dst,
                      Q_INT32 dstRowStride,
                      const Q_UINT8 *src,
                      Q_INT32 srcRowStride,
                      const Q_UINT8 *srcAlphaMask,
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
        compositeOver(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_COPY:
        compositeCopy(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
        break;
    case COMPOSITE_ERASE:
        compositeErase(dst, dstRowStride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
        break;

    case COMPOSITE_NO:
        // No composition.
        break;
    default:
        break;
    }
}

KisCompositeOpList KisTestColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));

    return list;
}

QString KisTestColorSpace::channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());

    const testcspixel *pix = (const testcspixel *)pixel;

    switch (channelIndex) {
    case 2:
        return QString().setNum(pix -> bmg);
    case 1:
        return QString().setNum(pix -> g);
    case 0:
        return QString().setNum(pix -> r);
    case 3:
        return QString().setNum(pix -> alpha);
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString KisTestColorSpace::normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());

    const testcspixel *pix = (const testcspixel *)pixel;

    switch (channelIndex) {
    case 2:
        return QString().setNum(static_cast<float>(pix -> bmg) / UINT16_MAX);
    case 1:
        return QString().setNum(static_cast<float>(pix -> g) / UINT8_MAX);
    case 0:
        return QString().setNum(static_cast<float>(pix -> r) / UINT8_MAX);
    case 3:
        return QString().setNum(static_cast<float>(pix -> alpha) / UINT8_MAX);
    default:
        Q_ASSERT(false);
        return QString();
    }
}

