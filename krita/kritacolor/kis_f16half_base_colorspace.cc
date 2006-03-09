/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#include "kdebug.h"

#include "kis_global.h"
#include "kis_f16half_base_colorspace.h"

Q_UINT8 KisF16HalfBaseColorSpace::getAlpha(const Q_UINT8 * U8_pixel) const
{
    if (m_alphaPos < 0) return OPACITY_OPAQUE;

    U8_pixel += m_alphaPos;

    const half *pixel = reinterpret_cast<const half *>(U8_pixel);
    return HALF_TO_UINT8(*pixel);
}

void KisF16HalfBaseColorSpace::setAlpha(Q_UINT8 *U8_pixel, Q_UINT8 alpha, Q_INT32 nPixels) const
{
    if (m_alphaPos < 0) return;
    Q_INT32 psize = pixelSize();

    while (nPixels > 0) {

        half *pixel = reinterpret_cast<half *>(U8_pixel + m_alphaPos);
        *pixel = UINT8_TO_HALF(alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KisF16HalfBaseColorSpace::multiplyAlpha(Q_UINT8 *U8_pixel, Q_UINT8 U8_alpha, Q_INT32 nPixels)
{
    if (m_alphaPos < 0) return;
    Q_INT32 psize = pixelSize();
    half alpha = UINT8_TO_HALF(U8_alpha);

    while (nPixels > 0) {

        half *pixelAlpha = reinterpret_cast<half *>(U8_pixel + m_alphaPos);
        *pixelAlpha *= alpha;

        --nPixels;
        U8_pixel += psize;
    }
}

void KisF16HalfBaseColorSpace::applyAlphaU8Mask(Q_UINT8 * U8_pixel, Q_UINT8 * alpha8, Q_INT32 nPixels)
{
    if (m_alphaPos < 0) return;

    Q_INT32 psize = pixelSize();

    while (nPixels--) {

        half *pixelAlpha = reinterpret_cast<half *>(U8_pixel + m_alphaPos);
        *pixelAlpha *= UINT8_TO_HALF(*alpha8);

        ++alpha8;
        U8_pixel += psize;
    }
}

void KisF16HalfBaseColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * U8_pixels, Q_UINT8 * alpha8, Q_INT32 nPixels)
{
    if (m_alphaPos < 0) return;

    Q_INT32 psize = pixelSize();

    while (nPixels--) {

            half *pixelAlpha = reinterpret_cast<half *>(U8_pixels + m_alphaPos);
            *pixelAlpha *= UINT8_TO_HALF(MAX_SELECTED - *alpha8);

            U8_pixels += psize;
            ++alpha8;
    }
}

QString KisF16HalfBaseColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < (Q_UINT32)nChannels());
    const half *pixel = reinterpret_cast<const half *>(U8_pixel);
    Q_UINT32 channelPosition = channels()[channelIndex] -> pos() / sizeof(half);

    return QString().setNum(pixel[channelPosition]);
}

QString KisF16HalfBaseColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < (Q_UINT32)nChannels());
    const half *pixel = reinterpret_cast<const half *>(U8_pixel);
    Q_UINT32 channelPosition = channels()[channelIndex] -> pos() / sizeof(half);

    return QString().setNum(100.0 * pixel[channelPosition]);
}

Q_UINT8 KisF16HalfBaseColorSpace::scaleToU8(const Q_UINT8 * U8_pixel, Q_INT32 channelPos)
{
    const half *pixelChannel = reinterpret_cast<const half *>(U8_pixel + channelPos);
    return HALF_TO_UINT8(*pixelChannel);
}

Q_UINT16 KisF16HalfBaseColorSpace::scaleToU16(const Q_UINT8 * U8_pixel, Q_INT32 channelPos)
{
    const half *pixelChannel = reinterpret_cast<const half *>(U8_pixel + channelPos);
    return HALF_TO_UINT16(*pixelChannel);
}

