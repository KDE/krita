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

#ifndef KIS_YCBCR_U8_COLORSPACE_H
#define KIS_YCBCR_U8_COLORSPACE_H

#include "klocale.h"
#include "kis_ycbcr_base_colorspace.h"
#include <KoColorModelStandardIds.h>
#include <KoCtlColorProfile.h>

#include "kis_ycbcr_traits.h"

typedef KoYCbCrTraits<quint8> YCbCrU8Traits;

class KisYCbCrU8ColorSpace : public KisYCbCrBaseColorSpace<YCbCrU8Traits>
{
public:
    KisYCbCrU8ColorSpace(const KoCtlColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence /*independence*/) const {
        return false;
    }
    virtual KoID colorModelId() const {
        return YCbCrAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }
    virtual KoColorSpace* clone() const;
};

class KisYCbCrU8ColorSpaceFactory : public KoCtlColorSpaceFactory
{
public:
    virtual QString id() const {
        return "YCbCrAU8";
    }
    virtual QString name() const {
        return i18n("YCBCR (8-bit integer/channel)");
    }
    virtual KoID colorModelId() const {
        return YCbCrAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }
    virtual bool userVisible() const {
        return true;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile * p) const {
        return new KisYCbCrU8ColorSpace(dynamic_cast<const KoCtlColorProfile*>(p));
    }

    virtual int referenceDepth() const {
        return 8;
    }
    virtual QString colorSpaceEngine() const {
        return "";
    }
    virtual bool isHdr() const {
        return false;
    }

    virtual QString defaultProfile() const {
        return "Standard YCbCr (8-bits)";
    }
};

#endif // KIS_STRATEGY_COLORSPACE_RGB_H_

