/*
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

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include <config.h>

#include LCMS_HEADER

#include "kis_alpha_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_channelinfo.h"
#include "kis_id.h"
#include "kis_integer_maths.h"

namespace {
    const Q_UINT8 PIXEL_MASK = 0;
}

KisAlphaColorSpace::KisAlphaColorSpace(KisColorSpaceFactoryRegistry * parent,
                                       KisProfile *p) :
    KisU8BaseColorSpace(KisID("ALPHA", i18n("Alpha mask")),  TYPE_GRAY_8, icSigGrayData, parent, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), i18n("A"), 0, KisChannelInfo::ALPHA, KisChannelInfo::UINT8));
    m_alphaPos = 0;
}

KisAlphaColorSpace::~KisAlphaColorSpace()
{
}

void KisAlphaColorSpace::fromQColor(const QColor& /*c*/, Q_UINT8 *dst, KisProfile * /*profile*/)
{
    dst[PIXEL_MASK] = OPACITY_OPAQUE;
}

void KisAlphaColorSpace::fromQColor(const QColor& /*c*/, Q_UINT8 opacity, Q_UINT8 *dst, KisProfile * /*profile*/)
{
    dst[PIXEL_MASK] = opacity;
}

void KisAlphaColorSpace::getAlpha(const Q_UINT8 *pixel, Q_UINT8 *alpha) const
{
    *alpha = *pixel;
}

void KisAlphaColorSpace::toQColor(const Q_UINT8 */*src*/, QColor *c, KisProfile * /*profile*/)
{
    c->setRgb(255, 255, 255);
}

void KisAlphaColorSpace::toQColor(const Q_UINT8 *src, QColor *c, Q_UINT8 *opacity, KisProfile * /*profile*/)
{
    c->setRgb(255, 255, 255);
    *opacity = src[PIXEL_MASK];
}

Q_UINT8 KisAlphaColorSpace::difference(const Q_UINT8 *src1, const Q_UINT8 *src2)
{
    // Arithmetic operands smaller than int are converted to int automatically
    return QABS(src2[PIXEL_MASK] - src1[PIXEL_MASK]);
}

void KisAlphaColorSpace::mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const
{
    if (nColors > 0) {
        Q_UINT32 total = 0;

        while(nColors)
        {
            nColors--;
            total += *colors[nColors] * weights[nColors];
        }
        *dst = total / 255;
    }
}

QValueVector<KisChannelInfo *> KisAlphaColorSpace::channels() const
{
    return m_channels;
}

bool KisAlphaColorSpace::convertPixelsTo(const Q_UINT8 *src,
                     Q_UINT8 *dst, KisAbstractColorSpace * dstColorSpace,
                     Q_UINT32 numPixels,
                     Q_INT32 /*renderingIntent*/)
{
    // No lcms trickery here, we are only a opacity channel
    Q_INT32 size = dstColorSpace->pixelSize();

    Q_UINT32 j = 0;
    Q_UINT32 i = 0;

    while ( i < numPixels ) {

        dstColorSpace->fromQColor(Qt::red, OPACITY_OPAQUE - *(src + i), (dst + j));

        i += 1;
        j += size;

    }
    return true;

}


//XXX bitblt of ColorSpaceAlpha does not take mask into consideration as this is probably not
// used ever
void KisAlphaColorSpace::bitBlt(Q_UINT8 *dst,
                Q_INT32 dststride,
                const Q_UINT8 *src,
                Q_INT32 srcRowStride,
                const Q_UINT8 *srcAlphaMask,
                Q_INT32 maskRowStride,
                Q_UINT8 opacity,
                Q_INT32 rows,
                Q_INT32 cols,
                const KisCompositeOp& op)
{

    Q_UINT8 *d;
    const Q_UINT8 *s;
     Q_INT32 i;
    Q_INT32 linesize;

    if (rows <= 0 || cols <= 0)
        return;
    switch (op.op()) {
    case COMPOSITE_COPY:
        compositeCopy(dst, dststride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
        return;
    case COMPOSITE_CLEAR:
        linesize = sizeof(Q_UINT8) * cols;
        d = dst;
        while (rows-- > 0) {
            memset(d, OPACITY_TRANSPARENT, linesize);
            d += dststride;
        }
        return;
    case COMPOSITE_ERASE:
        while (rows-- > 0) {
            d = dst;
            s = src;

            for (i = cols; i > 0; i--, d ++, s ++) {
                if (d[PIXEL_MASK] < s[PIXEL_MASK]) {
                    continue;
                }
                else {
                    d[PIXEL_MASK] = s[PIXEL_MASK];
                }

            }

            dst += dststride;
            src += srcRowStride;
        }
        return;
    case COMPOSITE_SUBTRACT:
        while (rows-- > 0) {
            d = dst;
            s = src;

            for (i = cols; i > 0; i--, d++, s++) {
                if (d[PIXEL_MASK] <= s[PIXEL_MASK]) {
                    d[PIXEL_MASK] = MIN_SELECTED;
                } else {
                    d[PIXEL_MASK] -= s[PIXEL_MASK];
                }
            }

            dst += dststride;
            src += srcRowStride;
        }
        return;
    case COMPOSITE_OVER:
    default:
        if (opacity == OPACITY_TRANSPARENT)
            return;
        if (opacity != OPACITY_OPAQUE) {
            while (rows-- > 0) {
                d = dst;
                s = src;
                for (i = cols; i > 0; i--, d++, s++) {
                    if (s[PIXEL_MASK] == OPACITY_TRANSPARENT)
                        continue;
                    int srcAlpha = (s[PIXEL_MASK] * opacity + UINT8_MAX / 2) / UINT8_MAX;
                    d[PIXEL_MASK] = (d[PIXEL_MASK] * (UINT8_MAX - srcAlpha) + srcAlpha * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
                }
                dst += dststride;
                src += srcRowStride;
            }
        }
        else {
            while (rows-- > 0) {
                d = dst;
                s = src;
                for (i = cols; i > 0; i--, d++, s++) {
                    if (s[PIXEL_MASK] == OPACITY_TRANSPARENT)
                        continue;
                    if (d[PIXEL_MASK] == OPACITY_TRANSPARENT || s[PIXEL_MASK] == OPACITY_OPAQUE) {
                        memcpy(d, s, 1);
                        continue;
                    }
                    int srcAlpha = s[PIXEL_MASK];
                    d[PIXEL_MASK] = (d[PIXEL_MASK] * (UINT8_MAX - srcAlpha) + srcAlpha * UINT8_MAX + UINT8_MAX / 2) / UINT8_MAX;
                }
                dst += dststride;
                src += srcRowStride;
            }
        }

    }
}

KisCompositeOpList KisAlphaColorSpace::userVisiblecompositeOps() const
{
    KisCompositeOpList list;

    list.append(KisCompositeOp(COMPOSITE_OVER));

    return list;
}

QString KisAlphaColorSpace::channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    Q_UINT32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KisAlphaColorSpace::normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    Q_UINT32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}


void KisAlphaColorSpace::convolveColors(Q_UINT8** colors, Q_INT32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags, Q_UINT8 *dst, Q_INT32 factor, Q_INT32 offset, Q_INT32 nColors) const
{
    Q_INT32 totalAlpha = 0;

    while (nColors--)
    {
        Q_INT32 weight = *kernelValues;

        if (weight != 0) {
            totalAlpha += (*colors)[PIXEL_MASK] * weight;
        }
        colors++;
        kernelValues++;
    }

    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_MASK] = CLAMP((totalAlpha/ factor) + offset, 0, Q_UINT8_MAX);
    }
}
