/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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

#ifndef KIS_STRATEGY_COLORSPACE_XYZ_U16_H_
#define KIS_STRATEGY_COLORSPACE_XYZ_U16_H_

#include <KoLcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

#include <pigment_xyz_u16_export.h>
#include <KoColorModelStandardIds.h>

typedef KoXyzTraits<quint16> XyzU16Traits;

class PIGMENT_XYZ_U16_EXPORT KisXyzU16ColorSpace : public KoLcmsColorSpace<XyzU16Traits>
{
    public:
        KisXyzU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence) const;
        virtual KoID colorModelId() const { return XYZAColorModelID; }
        virtual KoID colorDepthId() const { return Integer16BitsColorDepthID; }
};

#define TYPE_XYZA_16 (COLORSPACE_SH(PT_XYZ)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1))

class KisXyzU16ColorSpaceFactory : public KoLcmsColorSpaceFactory
{
    public:
        KisXyzU16ColorSpaceFactory() : KoLcmsColorSpaceFactory(TYPE_XYZA_16, icSigXYZData)
        {
        }
        virtual QString id() const { return "XYZA16"; }
        virtual QString name() const { return i18n("XYZ (16-bit integer/channel)"); }

        virtual KoID colorModelId() const { return XYZAColorModelID; }
        virtual KoID colorDepthId() const { return Integer16BitsColorDepthID; }

        virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) const { return new KisXyzU16ColorSpace(parent, p); }

        virtual QString defaultProfile() const { return "XYZ built-in - (lcms internal)"; }
};


#endif
