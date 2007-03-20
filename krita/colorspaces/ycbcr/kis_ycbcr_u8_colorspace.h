/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_YCBCR_U8_COLORSPACE_H
#define KIS_YCBCR_U8_COLORSPACE_H

#include "klocale.h"
#include "kis_ycbcr_base_colorspace.h"

#include "kis_ycbcr_traits.h"

typedef KoYCbCrTraits<Q_UINT8> YCbCrU8Traits;

class KisYCbCrU8ColorSpace : public KisYCbCrBaseColorSpace<YCbCrU8Traits>
{
    public:
        KisYCbCrU8ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence) const {
            return false;
        }
};

class KisYCbCrU8ColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("YCbCrAU8", i18n("YCBCR (8-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
   virtual quint32 colorSpaceType() { return TYPE_YCbCr_8; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigYCbCrData; };

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) { return new KisYCbCrU8ColorSpace(parent, p); };

    virtual QString defaultProfile() { return ""; };
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_

