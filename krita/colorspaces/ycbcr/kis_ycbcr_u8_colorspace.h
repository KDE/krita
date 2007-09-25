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
#include <KoColorModelStandardIds.h>

#include "kis_ycbcr_traits.h"

typedef KoYCbCrTraits<quint8> YCbCrU8Traits;

class KisYCbCrU8ColorSpace : public KisYCbCrBaseColorSpace<YCbCrU8Traits>
{
    public:
        KisYCbCrU8ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence /*independence*/) const {
            return false;
        }
        virtual KoID colorModelId() const { return YCbCrAColorModelID; }
        virtual KoID colorDepthId() const { return Integer8BitsColorDepthID; }
};

class KisYCbCrU8ColorSpaceFactory : public KoColorSpaceFactory
{
public:
    virtual QString id() const { return "YCbCrAU8"; }
    virtual QString name() const { return i18n("YCBCR (8-bit integer/channel)"); }
    virtual KoID colorModelId() const { return YCbCrAColorModelID; }
    virtual KoID colorDepthId() const { return Integer8BitsColorDepthID; }

    virtual bool profileIsCompatible(KoColorProfile* /*profile*/) const
    {
        return false;
    }

    virtual KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p) const { return new KisYCbCrU8ColorSpace(parent, p); }

    virtual bool isIcc() const { return false; }
    virtual bool isHdr() const { return false; }
    virtual QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;

    virtual QString defaultProfile() const { return ""; }
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_

