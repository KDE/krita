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

#include <QColor>

#include <kdebug.h>

#include "kis_abstract_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_integer_maths.h"

quint8 KisU8BaseColorSpace::getAlpha(const quint8 * pixel) const
{
    return pixel[m_alphaPos];
}



void KisU8BaseColorSpace::setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();

    pixels += m_alphaPos;
    while (nPixels > 0) {
        *pixels = alpha;
        --nPixels;
        pixels += psize;
    }

}

void KisU8BaseColorSpace::multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels)
{
    if (m_alphaPos < 0) return;
    qint32 psize = pixelSize();

    while (nPixels > 0) {
        pixels[m_alphaPos] = UINT8_MULT(pixels[m_alphaPos], alpha);
        --nPixels;
        pixels += psize;
    }
}

void KisU8BaseColorSpace::applyAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels)
{
    qint32 psize = pixelSize();

    while (nPixels--) {

        pixels[m_alphaPos] = UINT8_MULT(*(pixels + m_alphaPos) , *alpha);

        alpha++;
        pixels += psize;

    }
}

void KisU8BaseColorSpace::applyInverseAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels)
{
    qint32 psize = pixelSize();

    while(nPixels--) {

            quint16 p_alpha, s_alpha;

            p_alpha = getAlpha(pixels);
            s_alpha = MAX_SELECTED - *alpha;

            setAlpha(pixels, UINT8_MULT(p_alpha, s_alpha), 1);

            pixels += psize;
            ++alpha;
    }
}

QString KisU8BaseColorSpace::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    quint32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KisU8BaseColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < (quint32)nChannels());
    quint32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(100.0 * static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}


quint8 KisU8BaseColorSpace::scaleToU8(const quint8 * pixel, qint32 channelPos)
{
    return pixel[channelPos];
}

quint16 KisU8BaseColorSpace::scaleToU16(const quint8 * pixel, qint32 channelPos)
{
    return UINT8_TO_UINT16(pixel[channelPos]);
}

