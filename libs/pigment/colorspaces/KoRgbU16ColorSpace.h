/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KORGBU16COLORSPACE_H_
#define KORGBU16COLORSPACE_H_

#include <KoLcmsColorSpace.h>

struct RgbU16Traits {
    typedef quint16 channels_type;
    static const quint32 channels_nb = 4;
    static const qint32 alpha_pos = 3;
};

class KoRgbU16ColorSpace : public KoLcmsColorSpace<RgbU16Traits>
{
    public:
        KoRgbU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence) const;
};

class KoRgbU16ColorSpaceFactory : public KoColorSpaceFactory
{
    public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
        virtual KoID id() const { return KoID("RGBU16", i18n("RGB (16-bit integer/channel)")); };

    /**
         * lcms colorspace type definition.
     */
        virtual quint32 colorSpaceType() { return TYPE_BGRA_16; };

        virtual icColorSpaceSignature colorSpaceSignature() { return icSigRgbData; };
    
        virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KoRgbU16ColorSpace(parent, p); };

        virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};


#endif
