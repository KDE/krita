/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_YCBCR_U16_COLORSPACE_H
#define KIS_YCBCR_U16_COLORSPACE_H

#include "klocale.h"
#include "kis_ycbcr_base_colorspace.h"

#include "kis_ycbcr_traits.h"
#include <KoColorModelStandardIds.h>
#include <KoCtlColorProfile.h>

typedef KoYCbCrTraits<quint16> YCbCrU16Traits;

class KisYCbCrU16ColorSpace : public KisYCbCrBaseColorSpace<YCbCrU16Traits>
{
    public:
        KisYCbCrU16ColorSpace( const KoCtlColorProfile *p);
        virtual bool willDegrade(ColorSpaceIndependence independence) const {
          if (independence == TO_RGBA8 )
            return true;
          else
            return false;
        }
        virtual KoID colorModelId() const { return YCbCrAColorModelID; }
        virtual KoID colorDepthId() const { return Integer16BitsColorDepthID; }
        virtual KoColorSpace* clone() const;
};

class KisYCbCrU16ColorSpaceFactory : public KoCtlColorSpaceFactory
{
public:
    virtual QString id() const { return "YCbCrAU16"; }
    virtual QString name() const { return i18n("YCBCR (16-bit integer/channel)"); }
    virtual KoID colorModelId() const { return YCbCrAColorModelID; }
    virtual KoID colorDepthId() const { return Integer16BitsColorDepthID; }
    virtual bool userVisible() const { return true; }
    
    virtual KoColorSpace *createColorSpace( const KoColorProfile * p) const
    {
        return new KisYCbCrU16ColorSpace( dynamic_cast<const KoCtlColorProfile*>(p));
    }
    virtual int referenceDepth() const { return 16; }

    
    virtual QString colorSpaceEngine() const { return ""; }
    virtual bool isHdr() const { return false; }
    
    virtual QString defaultProfile() const { return "Standard YCbCr (16-bits)"; }
};

#endif

