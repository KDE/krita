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
#include "kis_lcms_base_colorspace.h"
#include "kis_integer_maths.h"
#include "kis_u16_base_colorspace.h"


quint8 KisU16BaseColorSpace::getAlpha(const quint8 * U8_pixel) const
{
    if (m_alphaPos < 0) return OPACITY_OPAQUE;

    U8_pixel+= m_alphaPos;

    const quint16 *pixel = reinterpret_cast<const quint16 *>(U8_pixel);
    return UINT16_TO_UINT8(*pixel);
}


void KisU16BaseColorSpace::setAlpha(quint8 *U8_pixel, quint8 alpha, qint32 nPixels) const
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();


    while (nPixels > 0) {

        quint16 *pixel = reinterpret_cast<quint16 *>(U8_pixel + m_alphaPos);
        pixel[0] = UINT8_TO_UINT16(alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KisU16BaseColorSpace::multiplyAlpha(quint8 *U8_pixel, quint8 U8_alpha, qint32 nPixels)
{
    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();
    quint16 alpha = UINT8_TO_UINT16(U8_alpha);

    while (nPixels > 0) {

        quint16 *pixelAlpha = reinterpret_cast<quint16 *>(U8_pixel + m_alphaPos);
        *pixelAlpha = UINT16_MULT(*pixelAlpha, alpha);

        --nPixels;
        U8_pixel += psize;
    }
}

void KisU16BaseColorSpace::applyAlphaU8Mask(quint8 * U8_pixel, quint8 * alpha8, qint32 nPixels)
{
    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();

    while (nPixels--) {

        // Go to the alpha position (which is given in bytes from the start of the pixel,
        // and cast to short.

        quint16 *pixelAlpha = reinterpret_cast<quint16 *>(U8_pixel + m_alphaPos);
        *pixelAlpha = UINT8_MULT(*pixelAlpha, *alpha8);

        ++alpha8;
        U8_pixel += psize;

    }
}

void KisU16BaseColorSpace::applyInverseAlphaU8Mask(quint8 * U8_pixels, quint8 * alpha8, qint32 nPixels)
{

    if (m_alphaPos < 0) return;

    qint32 psize = pixelSize();


    while(nPixels--) {

            quint16 s_alpha8;
            quint32 p_alpha, s_alpha16;

            quint16 *alpha = reinterpret_cast<quint16 *>(U8_pixels + m_alphaPos);

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

QString KisU16BaseColorSpace::channelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    const quint16 *pixel = reinterpret_cast<const quint16 *>(U8_pixel);
    quint32 channelPosition = channels()[channelIndex]->pos() / sizeof(quint16);

    return QString().setNum(pixel[channelPosition]);
}

QString KisU16BaseColorSpace::normalisedChannelValueText(const quint8 *U8_pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    const quint16 *pixel = reinterpret_cast<const quint16 *>(U8_pixel);
    quint32 channelPosition = m_channels[channelIndex]->pos() / sizeof(quint16);

    return QString().setNum(100.0 * static_cast<float>(pixel[channelPosition]) / UINT16_MAX);
}

quint8 KisU16BaseColorSpace::scaleToU8(const quint8 * U8_pixel, qint32 channelPos)
{
    const quint16 *pixel = reinterpret_cast<const quint16 *>(U8_pixel);
    return UINT16_TO_UINT8(pixel[channelPos]);
}

quint16 KisU16BaseColorSpace::scaleToU16(const quint8 * U8_pixel, qint32 channelPos)
{
    const quint16 *pixel = reinterpret_cast<const quint16 *>(U8_pixel);
    return pixel[channelPos];
}

