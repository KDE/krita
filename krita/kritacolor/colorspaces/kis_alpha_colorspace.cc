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

#include <QImage>

#include <kdebug.h>
#include <klocale.h>

#include <config.h>

#include <lcms.h>

#include "kis_alpha_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_channelinfo.h"
#include "kis_id.h"
#include "kis_integer_maths.h"

namespace {
    const quint8 PIXEL_MASK = 0;
}

KisAlphaColorSpace::KisAlphaColorSpace(KisColorSpaceFactoryRegistry * parent,
                                       KisProfile *p) :
    KisColorSpace(KisID("ALPHA", i18n("Alpha mask")),  parent)
    , KisU8BaseColorSpace(0)
    , KisLcmsBaseColorSpace(TYPE_GRAY_8, icSigGrayData, p)
{
    m_channels.push_back(new KisChannelInfo(i18n("Alpha"), 0, KisChannelInfo::ALPHA, KisChannelInfo::UINT8));
}

KisAlphaColorSpace::~KisAlphaColorSpace()
{
}

void KisAlphaColorSpace::fromQColor(const QColor& /*c*/, quint8 *dst, KisProfile * /*profile*/)
{
    dst[PIXEL_MASK] = OPACITY_OPAQUE;
}

void KisAlphaColorSpace::fromQColor(const QColor& /*c*/, quint8 opacity, quint8 *dst, KisProfile * /*profile*/)
{
    dst[PIXEL_MASK] = opacity;
}

void KisAlphaColorSpace::getAlpha(const quint8 *pixel, quint8 *alpha) const
{
    *alpha = *pixel;
}

void KisAlphaColorSpace::toQColor(const quint8 */*src*/, QColor *c, KisProfile * /*profile*/)
{
    c->setRgb(255, 255, 255);
}

void KisAlphaColorSpace::toQColor(const quint8 *src, QColor *c, quint8 *opacity, KisProfile * /*profile*/)
{
    c->setRgb(255, 255, 255);
    *opacity = src[PIXEL_MASK];
}

quint8 KisAlphaColorSpace::difference(const quint8 *src1, const quint8 *src2)
{
    // Arithmetic operands smaller than int are converted to int automatically
    return QABS(src2[PIXEL_MASK] - src1[PIXEL_MASK]);
}

void KisAlphaColorSpace::mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const
{
    if (nColors > 0) {
        quint32 total = 0;

        while(nColors)
        {
            nColors--;
            total += *colors[nColors] * weights[nColors];
        }
        *dst = total / 255;
    }
}

Q3ValueVector<KisChannelInfo *> KisAlphaColorSpace::channels() const
{
    return m_channels;
}

bool KisAlphaColorSpace::convertPixelsTo(const quint8 *src,
                     quint8 *dst, KisColorSpace * dstColorSpace,
                     quint32 numPixels,
                     qint32 /*renderingIntent*/)
{
    // No lcms trickery here, we are only a opacity channel
    qint32 size = dstColorSpace->pixelSize();

    quint32 j = 0;
    quint32 i = 0;

    while ( i < numPixels ) {

        dstColorSpace->fromQColor(Qt::red, OPACITY_OPAQUE - *(src + i), (dst + j));

        i += 1;
        j += size;

    }
    return true;

}


//XXX bitblt of ColorSpaceAlpha does not take mask into consideration as this is probably not
// used ever
void KisAlphaColorSpace::bitBlt(quint8 *dst,
                qint32 dststride,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KisCompositeOp& op)
{

    quint8 *d;
    const quint8 *s;
     qint32 i;
    qint32 linesize;

    if (rows <= 0 || cols <= 0)
        return;
    switch (op.op()) {
    case COMPOSITE_COPY:
        compositeCopy(dst, dststride, src, srcRowStride, srcAlphaMask, maskRowStride, rows, cols, opacity);
        return;
    case COMPOSITE_CLEAR:
        linesize = sizeof(quint8) * cols;
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

QString KisAlphaColorSpace::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    quint32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KisAlphaColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < nChannels());
    quint32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}


void KisAlphaColorSpace::convolveColors(quint8** colors, qint32 * kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nColors) const
{
    qint32 totalAlpha = 0;

    while (nColors--)
    {
        qint32 weight = *kernelValues;

        if (weight != 0) {
            totalAlpha += (*colors)[PIXEL_MASK] * weight;
        }
        colors++;
        kernelValues++;
    }

    if (channelFlags & KisChannelInfo::FLAG_ALPHA) {
        dst[PIXEL_MASK] = CLAMP((totalAlpha/ factor) + offset, 0, quint8_MAX);
    }
}
