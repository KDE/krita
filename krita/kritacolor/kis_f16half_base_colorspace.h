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
#ifndef KIS_F16HALF_BASE_COLORSPACE_H_
#define KIS_F16HALF_BASE_COLORSPACE_H_

#include <qcolor.h>

#include <half.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_integer_maths.h"

/**
 * This class is the base for all 16-bit float colorspaces using the
 * OpenEXR half format. This format can be used with the OpenGL
 * extensions GL_NV_half_float and GL_ARB_half_float_pixel.
 */

inline half UINT8_TO_HALF(uint c)
{
    return static_cast<half>(c) / UINT8_MAX;
}

inline uint HALF_TO_UINT8(half c)
{
    return static_cast<uint>(CLAMP(static_cast<int>(c * static_cast<int>(UINT8_MAX) + 0.5),
                                   static_cast<int>(UINT8_MIN), static_cast<int>(UINT8_MAX)));
}


inline uint HALF_TO_UINT16(half c)
{
    return static_cast<uint>(CLAMP(static_cast<int>(c * static_cast<int>(UINT16_MAX) + 0.5),
                                   static_cast<int>(UINT16_MIN), static_cast<int>(UINT16_MAX)));
}

inline half HALF_BLEND(half a, half b, half alpha)
{
    return (a - b) * alpha + b;
}

#define F16HALF_OPACITY_OPAQUE ((half)1.0f)
#define F16HALF_OPACITY_TRANSPARENT ((half)0.0f)

class KRITACOLOR_EXPORT KisF16HalfBaseColorSpace : public KisAbstractColorSpace {

public:

    KisF16HalfBaseColorSpace(const KisID & id, DWORD cmType, icColorSpaceSignature colorSpaceSignature,
                             KisColorSpaceFactoryRegistry * parent,
                             KisProfile *p)
	: KisAbstractColorSpace(id, cmType, colorSpaceSignature, parent, p)
    {
        m_alphaSize = sizeof(half);
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

    virtual bool hasHighDynamicRange() const { return true; }
};

#endif // KIS_F16HALF_BASE_COLORSPACE_H_
