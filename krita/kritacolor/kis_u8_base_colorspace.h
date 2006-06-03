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
#ifndef KIS_U8_BASE_COLORSPACE_H_
#define KIS_U8_BASE_COLORSPACE_H_

#include <QColor>

#include <QColor>

#include "kis_global.h"
#include "kis_colorspace.h"

/**
 * This class is the base for all homogenous 8-bit/channel colorspaces with 8-bit alpha channels
 */
class KRITACOLOR_EXPORT KisU8BaseColorSpace : public virtual KisColorSpace {

public:

    KisU8BaseColorSpace(qint32 alphaPos)
    {
        m_alphaPos = alphaPos;
    };

    virtual quint8 getAlpha(const quint8 * pixel) const;
    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const;
    virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels);

    virtual void applyAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels);
    virtual void applyInverseAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels);

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;

    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos);
    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos);

private:
    qint32 m_alphaPos; // The position in _bytes_ of the alpha channel
    //qint32 m_alphaSize; // The width in _bytes_ of the alpha channel

};


#endif // KIS_U8_BASE_COLORSPACE_H_
