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

class KisF16HalfBaseColorSpace : public KisAbstractColorSpace {

public:

    KisF16HalfBaseColorSpace(const KisID & id, DWORD cmType, icColorSpaceSignature colorSpaceSignature,
                             KisColorSpaceFactoryRegistry * parent,
                             KisProfile *p)
	: KisAbstractColorSpace(id, cmType, colorSpaceSignature, parent, p)
    {
        m_alphaSize = sizeof(half);
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

protected:
    // For Alpha Composite
    struct F16HalfMult {
        inline half operator()(const half& a, const half& b) const {
            return a * b;
        }
    };
    struct Uint8ToF16Half {
        inline half operator()(const Q_UINT8 src) const {
            return UINT8_TO_HALF(src);
        }
    };
    struct F16HalfOpacityTest {
        inline bool operator()(const half& opacity) const {
            return opacity > F16HALF_OPACITY_TRANSPARENT + HALF_EPSILON;
        }
    };
};

#endif // KIS_F16HALF_BASE_COLORSPACE_H_
