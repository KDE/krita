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

#ifndef KORGBU32COLORSPACE_H_
#define KORGBU32COLORSPACE_H_

#include <KoLcmsColorSpace.h>

struct RgbU32Traits {
    typedef quint32 channels_type;
    static const quint32 channels_nb = 4;
    static const qint32 alpha_pos = 3;
};

class KoRgbU32ColorSpace : public KoLcmsColorSpace<RgbU32Traits>
{
    public:
        KoRgbU32ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence);
};

#define TYPE_BGRA_32           (COLORSPACE_SH(PT_RGB)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(4)|DOSWAP_SH(1)|SWAPFIRST_SH(1))

class KoRgbU32ColorSpaceFactory : public KoColorSpaceFactory
{
    public:
    /**
     * Krita definition for use in .kra files and internally: unchanging name +
     * i18n'able description.
     */
        virtual KoID id() const { return KoID("RGBU32", i18n("RGB (32-bit integer/channel)")); };

    /**
         * lcms colorspace type definition.
     */
        virtual quint32 colorSpaceType() { return TYPE_BGRA_32; };

        virtual icColorSpaceSignature colorSpaceSignature() { return icSigLabData; };
    
        virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) { return new KoRgbU32ColorSpace(parent, p); };

        virtual QString defaultProfile() { return "sRGB built-in - (lcms internal)"; };
};


#endif
