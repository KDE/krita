/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_STRATEGY_COLORSPACE_RGB_16F_HDR_H_
#define KIS_STRATEGY_COLORSPACE_RGB_16F_HDR_H_

#include "klocale.h"
#include "kis_rgb_float_hdr_colorspace.h"

#include <half.h>

struct RgbF16Traits {
    typedef half channels_type;
    static const quint32 channels_nb = 4;
    static const qint32 alpha_pos = 3;
    static const qint32 red = 2;
    static const qint32 green = 1;
    static const qint32 blue = 0;
};

class KisRgbF16HDRColorSpace : public KisRgbFloatHDRColorSpace<RgbF16Traits>
{
    public:
        KisRgbF16HDRColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence) const {
          if (independence == TO_RGBA8 /*|| independence == TO_LAB16*/)
            return true;
          else
            return false;
        }
};

// FIXME: lcms doesn't support 16-bit float
#define RGBAF16HALF_LCMS_TYPE TYPE_BGRA_16

class KisRgbF16HDRColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("RGBAF16HALF", i18n("RGB (16-bit float/channel) for High Dynamic Range imaging")); };

    /**
     * lcms colorspace type definition.
     */
   virtual quint32 colorSpaceType() { return RGBAF16HALF_LCMS_TYPE; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigRgbData; };

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) { return new KisRgbF16HDRColorSpace(parent, p); };

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
