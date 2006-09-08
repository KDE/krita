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

#include <qcolor.h>

#include <qcolor.h>

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_integer_maths.h"

/**
 * This class is the base for all homogenous 8-bit/channel colorspaces with 8-bit alpha channels
 */
class KisU8BaseColorSpace : public KisAbstractColorSpace {

public:

    KisU8BaseColorSpace(const KisID & id, DWORD cmType, icColorSpaceSignature colorSpaceSignature,
                        KisColorSpaceFactoryRegistry * parent,
                        KisProfile *p)
	: KisAbstractColorSpace(id, cmType, colorSpaceSignature, parent, p)
    {
	m_alphaSize = sizeof(Q_UINT8);
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

protected:
    // For Alpha Composite
    struct U8Mult {
        inline Q_UINT8 operator()(const Q_UINT8& a, const Q_UINT8& b) const {
            return UINT8_MULT(a, b);
        }
    };
    struct Uint8ToU8 {
        inline Q_UINT8 operator()(const Q_UINT8 src) const {
            return src;
        }
    };
    struct U8OpacityTest {
        inline bool operator()(const Q_UINT8& opacity) const {
            return opacity != OPACITY_TRANSPARENT;
        }
    };
};


#endif // KIS_U8_BASE_COLORSPACE_H_
