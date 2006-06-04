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

#include "KoF16HalfColorSpaceTrait.h"

quint8 KoF16HalfColorSpaceTrait::getAlpha(const quint8 * U8_pixel) const
{
    if (m_alphaPos < 0) return OPACITY_OPAQUE;

    U8_pixel += m_alphaPos;

    const half *pixel = reinterpret_cast<const half *>(U8_pixel);
    return HALF_TO_UINT8(*pixel);
}

void KoF16HalfColorSpaceTrait::setAlpha(quint8 *U8_pixel, quint8 alpha, qint32 nPixels) const
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();

    while (nPixels > 0) {

        half *pixel = reinterpret_cast<half *>(U8_pixel + m_alphaPos);
        *pixel = UINT8_TO_HALF(alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KoF16HalfColorSpaceTrait::multiplyAlpha(quint8 *U8_pixel, quint8 U8_alpha, qint32 nPixels)
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();
    half alpha = UINT8_TO_HALF(U8_alpha);

    while (nPixels > 0) {

        half *pixelAlpha = reinterpret_cast<half *>(U8_pixel + m_alphaPos);
        *pixelAlpha *= alpha;

        --nPixels;
        U8_pixel += psize;
    }
}

void KoF16HalfColorSpaceTrait::applyAlphaU8Mask(quint8 * U8_pixel, quint8 * alpha8, qint32 nPixels)
{
    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();

    while (nPixels--) {

        half *pixelAlpha = reinterpret_cast<half *>(U8_pixel + m_alphaPos);
        *pixelAlpha *= UINT8_TO_HALF(*alpha8);

        ++alpha8;
        U8_pixel += psize;
    }
}

void KoF16HalfColorSpaceTrait::applyInverseAlphaU8Mask(quint8 * U8_pixels, quint8 * alpha8, qint32 nPixels)
{
    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();

    while (nPixels--) {

            half *pixelAlpha = reinterpret_cast<half *>(U8_pixels + m_alphaPos);
            *pixelAlpha *= UINT8_TO_HALF(OPACITY_OPAQUE - *alpha8);

            U8_pixels += psize;
            ++alpha8;
    }
}

QString KoF16HalfColorSpaceTrait::channelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    const half *pixel = reinterpret_cast<const half *>(U8_pixel);
    quint32 channelPosition = channels()[channelIndex] -> pos() / sizeof(half);

    return QString().setNum(pixel[channelPosition]);
}

QString KoF16HalfColorSpaceTrait::normalisedChannelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    const half *pixel = reinterpret_cast<const half *>(U8_pixel);
    quint32 channelPosition = channels()[channelIndex] -> pos() / sizeof(half);

    return QString().setNum(100.0 * pixel[channelPosition]);
}

quint8 KoF16HalfColorSpaceTrait::scaleToU8(const quint8 * U8_pixel, qint32 channelPos)
{
    const half *pixelChannel = reinterpret_cast<const half *>(U8_pixel + channelPos);
    return HALF_TO_UINT8(*pixelChannel);
}

quint16 KoF16HalfColorSpaceTrait::scaleToU16(const quint8 * U8_pixel, qint32 channelPos)
{
    const half *pixelChannel = reinterpret_cast<const half *>(U8_pixel + channelPos);
    return HALF_TO_UINT16(*pixelChannel);
}

