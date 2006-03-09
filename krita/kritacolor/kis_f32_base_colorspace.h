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
#ifndef KIS_F32_BASE_COLORSPACE_H_
#define KIS_F32_BASE_COLORSPACE_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_integer_maths.h"

/**
 * This class is the base for all 32-bit float colorspaces.
 */

inline float UINT8_TO_FLOAT(uint c)
{
    return static_cast<float>(c) / UINT8_MAX;
}

inline uint FLOAT_TO_UINT8(float c)
{
    return static_cast<uint>(CLAMP(static_cast<int>(c * static_cast<int>(UINT8_MAX) + 0.5),
                                   static_cast<int>(UINT8_MIN), static_cast<int>(UINT8_MAX)));
}


inline uint FLOAT_TO_UINT16(float c)
{
    return static_cast<uint>(CLAMP(static_cast<int>(c * static_cast<int>(UINT16_MAX) + 0.5),
                                   static_cast<int>(UINT16_MIN), static_cast<int>(UINT16_MAX)));
}

inline float FLOAT_BLEND(float a, float b, float alpha)
{
    return (a - b) * alpha + b;
}

#define F32_OPACITY_OPAQUE 1.0f
#define F32_OPACITY_TRANSPARENT 0.0f

class KisF32BaseColorSpace : public KisAbstractColorSpace {

public:

    KisF32BaseColorSpace(const KisID & id, DWORD cmType, icColorSpaceSignature colorSpaceSignature, KisColorSpaceFactoryRegistry * parent, KisProfile *p)
	: KisAbstractColorSpace(id, cmType, colorSpaceSignature, parent, p)
    {
	m_alphaSize = sizeof(float);
    };

    virtual Q_UINT8 getAlpha(const Q_UINT8 * pixel) const;
    virtual void setAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels) const;
    virtual void multiplyAlpha(Q_UINT8 * pixels, Q_UINT8 alpha, Q_INT32 nPixels);

    virtual void applyAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels);
    virtual void applyInverseAlphaU8Mask(Q_UINT8 * pixels, Q_UINT8 * alpha, Q_INT32 nPixels);

    virtual QString channelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;
    virtual QString normalisedChannelValueText(const Q_UINT8 *pixel, Q_UINT32 channelIndex) const;

    virtual Q_UINT8 scaleToU8(const Q_UINT8 * srcPixel, Q_INT32 channelPos);
    virtual Q_UINT16 scaleToU16(const Q_UINT8 * srcPixel, Q_INT32 channelPos);

    virtual bool hasHighDynamicRange() const { return true; }
};

#endif // KIS_F32_BASE_COLORSPACE_H_
