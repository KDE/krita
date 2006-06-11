/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "kdebug.h"

#include "KoF32HalfColorSpaceTrait.h"

quint8 KoF32ColorSpaceTrait::getAlpha(const quint8 * U8_pixel) const
{
    if (m_alphaPos < 0) return OPACITY_OPAQUE;

    U8_pixel += m_alphaPos;

    const float *pixel = reinterpret_cast<const float *>(U8_pixel);
    return FLOAT_TO_UINT8(*pixel);
}

void KoF32ColorSpaceTrait::setAlpha(quint8 *U8_pixel, quint8 alpha, qint32 nPixels) const
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();

    while (nPixels > 0) {

        float *pixel = reinterpret_cast<float *>(U8_pixel + m_alphaPos);
        *pixel = UINT8_TO_FLOAT(alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KoF32ColorSpaceTrait::multiplyAlpha(quint8 *U8_pixel, quint8 U8_alpha, qint32 nPixels)
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();
    float alpha = UINT8_TO_FLOAT(U8_alpha);

    while (nPixels > 0) {

        float *pixelAlpha = reinterpret_cast<float *>(U8_pixel + m_alphaPos);
        *pixelAlpha *= alpha;

        --nPixels;
        U8_pixel += psize;
    }
}

void KoF32ColorSpaceTrait::applyAlphaU8Mask(quint8 * U8_pixel, quint8 * alpha8, qint32 nPixels)
{
    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();

    while (nPixels--) {

        float *pixelAlpha = reinterpret_cast<float *>(U8_pixel + m_alphaPos);
        *pixelAlpha *= UINT8_TO_FLOAT(*alpha8);

        ++alpha8;
        U8_pixel += psize;
    }
}

void KoF32ColorSpaceTrait::applyInverseAlphaU8Mask(quint8 * U8_pixels, quint8 * alpha8, qint32 nPixels)
{
    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();

    while (nPixels--) {

            float *pixelAlpha = reinterpret_cast<float *>(U8_pixels + m_alphaPos);
            *pixelAlpha *= UINT8_TO_FLOAT(OPACITY_OPAQUE - *alpha8);

            U8_pixels += psize;
            ++alpha8;
    }
}

QString KoF32ColorSpaceTrait::channelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    const float *pixel = reinterpret_cast<const float *>(U8_pixel);
    quint32 channelPosition = channels()[channelIndex]->pos() / sizeof(float);

    return QString().setNum(pixel[channelPosition]);
}

QString KoF32ColorSpaceTrait::normalisedChannelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    const float *pixel = reinterpret_cast<const float *>(U8_pixel);
    quint32 channelPosition = channels()[channelIndex]->pos() / sizeof(float);

    return QString().setNum(100.0 * pixel[channelPosition]);
}

quint8 KoF32ColorSpaceTrait::scaleToU8(const quint8 * U8_pixel, qint32 channelPos)
{
    const float *pixelChannel = reinterpret_cast<const float *>(U8_pixel + channelPos);
    return FLOAT_TO_UINT8(*pixelChannel);
}

quint16 KoF32ColorSpaceTrait::scaleToU16(const quint8 * U8_pixel, qint32 channelPos)
{
    const float *pixelChannel = reinterpret_cast<const float *>(U8_pixel + channelPos);
    return FLOAT_TO_UINT16(*pixelChannel);
}

