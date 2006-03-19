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

#include <qcolor.h>

#include <kdebug.h>

#include "kis_abstract_colorspace.h"
#include "kis_u8_base_colorspace.h"
#include "kis_integer_maths.h"

Q_UINT8 KisU8BaseColorSpace::getAlpha(const Q_UINT8 * pixel) const
{
    return pixel[m_alphaPos];
}



void KisU8BaseColorSpace::setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels) const
{
    if (m_alphaPos < 0) return;
    Q_INT32 psize = pixelSize();

    pixels += m_alphaPos;
    while (nPixels > 0) {
        *pixels = alpha;
        --nPixels;
        pixels += psize;
    }

}

void KisU8BaseColorSpace::multiplyAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels)
{
    if (m_alphaPos < 0) return;
    Q_INT32 psize = pixelSize();

    while (nPixels > 0) {
        pixels[m_alphaPos] = UINT8_MULT(pixels[m_alphaPos], alpha);
        --nPixels;
        pixels += psize;
    }
}

void KisU8BaseColorSpace::applyAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
    Q_INT32 psize = pixelSize();

    while (nPixels--) {

        pixels[m_alphaPos] = UINT8_MULT(*(pixels + m_alphaPos) , *alpha);

        alpha++;
        pixels += psize;

    }
}

void KisU8BaseColorSpace::applyInverseAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels)
{
    Q_INT32 psize = pixelSize();

    while(nPixels--) {

            Q_UINT16 p_alpha, s_alpha;

            p_alpha = getAlpha(pixels);
            s_alpha = MAX_SELECTED - *alpha;

            setAlpha(pixels, UINT8_MULT(p_alpha, s_alpha), 1);

            pixels += psize;
            ++alpha;
    }
}

QString KisU8BaseColorSpace::channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < (Q_UINT32)nChannels());
    Q_UINT32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KisU8BaseColorSpace::normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const
{
    Q_ASSERT(channelIndex < (Q_UINT32)nChannels());
    Q_UINT32 channelPosition = m_channels[channelIndex]->pos();

    return QString().setNum(100.0 * static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}


Q_UINT8 KisU8BaseColorSpace::scaleToU8(const Q_UINT8 * pixel, Q_INT32 channelPos)
{
    return pixel[channelPos];
}

Q_UINT16 KisU8BaseColorSpace::scaleToU16(const Q_UINT8 * pixel, Q_INT32 channelPos)
{
    return UINT8_TO_UINT16(pixel[channelPos]);
}

