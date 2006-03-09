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
#ifndef KIS_U16_BASE_COLORSPACE_H_
#define KIS_U16_BASE_COLORSPACE_H_

#include "kis_global.h"
#include "kis_abstract_colorspace.h"
#include "kis_integer_maths.h"

/**
 * This is the base class for 16-bit/channel colorspaces with 16-bit alpha
 * channels. It defines a number of common methods, like handling 16-bit alpha
 * and up- and down-scaling of channels.
 */
class KisU16BaseColorSpace : public KisAbstractColorSpace {

public:

    static const Q_UINT16 U16_OPACITY_OPAQUE = UINT16_MAX;
    static const Q_UINT16 U16_OPACITY_TRANSPARENT = UINT16_MIN;

public:

    KisU16BaseColorSpace(const KisID & id, DWORD cmType, icColorSpaceSignature colorSpaceSignature,
                         KisColorSpaceFactoryRegistry * parent,
                         KisProfile *p)
	: KisAbstractColorSpace(id, cmType, colorSpaceSignature,
                                parent,
                                p)
    {
	    m_alphaSize = sizeof(Q_UINT16);
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
};
#endif // KIS_U16_BASE_COLORSPACE_H_
