/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_abstract_colorspace.h"
#include "kis_integer_maths.h"
#include "kis_u16_base_colorspace.h"


Q_UINT8 KisU16BaseColorSpace::getAlpha(const Q_UINT8 * U8_pixel) const
{
    if (m_alphaPos < 0) return OPACITY_OPAQUE;

    U8_pixel+= m_alphaPos;

    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    return UINT16_TO_UINT8(*pixel);
}


void KisU16BaseColorSpace::setAlpha(Q_UINT8 *U8_pixel, Q_UINT8 alpha, Q_INT32 nPixels) const
{
    if (m_alphaPos < 0) return;
    Q_INT32 psize = pixelSize();


    while (nPixels > 0) {

        Q_UINT16 *pixel = reinterpret_cast<Q_UINT16 *>(U8_pixel + m_alphaPos);
        pixel[0] = UINT8_TO_UINT16(alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KisU16BaseColorSpace::multiplyAlpha(Q_UINT8 *U8_pixel, Q_UINT8 U8_alpha, Q_INT32 nPixels)
{
    if (m_alphaPos < 0) return;

    Q_INT32 psize = pixelSize();
    Q_UINT16 alpha = UINT8_TO_UINT16(U8_alpha);

    while (nPixels > 0) {

        Q_UINT16 *pixelAlpha = reinterpret_cast<Q_UINT16 *>(U8_pixel + m_alphaPos);
        *pixelAlpha = UINT16_MULT(*pixelAlpha, alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KisU16BaseColorSpace::applyAlphaU8Mask(Q_UINT8 * U8_pixel, Q_UINT8 * alpha8, Q_INT32 nPixels)
{
    if (m_alphaPos < 0) return;

    Q_INT32 psize = pixelSize();

    while (nPixels--) {

        // Go to the alpha position (which is given in bytes from the start of the pixel,
        // and cast to short.

        Q_UINT16 *pixelAlpha = reinterpret_cast<Q_UINT16 *>(U8_pixel + m_alphaPos);
        *pixelAlpha = UINT8_MULT(*pixelAlpha, *alpha8);

        ++alpha8;
        U8_pixel += psize;

    }
}

void KisU16BaseColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * U8_pixels, Q_UINT8 * alpha8, Q_INT32 nPixels)
{

    if (m_alphaPos < 0) return;

    Q_INT32 psize = pixelSize();


    while(nPixels--) {

            Q_UINT16 s_alpha8;
            Q_UINT32 p_alpha, s_alpha16;

            Q_UINT16 *alpha = reinterpret_cast<Q_UINT16 *>(U8_pixels + m_alphaPos);

            p_alpha = *(alpha);
            s_alpha8 = MAX_SELECTED - *alpha8;
            s_alpha16 = UINT8_TO_UINT16(s_alpha8);

            // Go to the alpha position (which is given in bytes from the start of the pixel,
            // and cast to short.

            alpha[0] = UINT16_MULT(p_alpha, s_alpha16);

            U8_pixels += psize;
            ++alpha8;
    }
}

QString KisU16BaseColorSpace::channelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < (Q_UINT32)nChannels());
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    Q_UINT32 channelPosition = channels()[channelIndex]->pos() / sizeof(Q_UINT16);

    return QString().setNum(pixel[channelPosition]);
}

QString KisU16BaseColorSpace::normalisedChannelValueText(const Q_UINT8 *U8_pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < (Q_UINT32)nChannels());
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    Q_UINT32 channelPosition = m_channels[channelIndex]->pos() / sizeof(Q_UINT16);

    return QString().setNum(100.0 * static_cast<float>(pixel[channelPosition]) / UINT16_MAX);
}

Q_UINT8 KisU16BaseColorSpace::scaleToU8(const Q_UINT8 * U8_pixel, Q_INT32 channelPos)
{
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    return UINT16_TO_UINT8(pixel[channelPos]);
}

Q_UINT16 KisU16BaseColorSpace::scaleToU16(const Q_UINT8 * U8_pixel, Q_INT32 channelPos)
{
    const Q_UINT16 *pixel = reinterpret_cast<const Q_UINT16 *>(U8_pixel);
    return pixel[channelPos];
}

