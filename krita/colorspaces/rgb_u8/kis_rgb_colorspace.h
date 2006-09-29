/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#ifndef KIS_STRATEGY_COLORSPACE_RGB_H_
#define KIS_STRATEGY_COLORSPACE_RGB_H_

#include "klocale.h"

#include "KoLcmsColorSpace.h"

struct RgbU8Traits {
    typedef quint8 channels_type;
    static const quint32 channels_nb = 4;
    static const qint32 alpha_pos = 3;
};

const quint8 PIXEL_BLUE = 0;
const quint8 PIXEL_GREEN = 1;
const quint8 PIXEL_RED = 2;
const quint8 PIXEL_ALPHA = RgbU8Traits::alpha_pos;
const qint32 MAX_CHANNEL_RGB = 3;
const qint32 MAX_CHANNEL_RGBA = 4;

class KisRgbColorSpace : public KoLcmsColorSpace<RgbU8Traits>
{
    public:
        KisRgbColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence ) const { return false; }
};

class KisRgbColorSpaceFactory : public KoColorSpaceFactory
{
public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const { return KoID("RGBA", i18n("RGB (8-bit integer/channel)")); };

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() { return TYPE_BGRA_8; };

    virtual icColorSpaceSignature colorSpaceSignature() { return icSigRgbData; };

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) { return new KisRgbColorSpace(parent, p); };

    virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_
