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

#ifndef KIS_STRATEGY_COLORSPACE_CMYK_U8_H_
#define KIS_STRATEGY_COLORSPACE_CMYK_U8_H_

#include <KoLcmsColorSpace.h>
#include <KoColorSpaceTraits.h>
#include "KoColorModelStandardIds.h"

typedef KoCmykTraits<quint8> CmykU8Traits;

class KoCmykU8ColorSpace : public KoLcmsColorSpace<CmykU8Traits>
{
public:
    KoCmykU8ColorSpace(KoColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence independence) const;
    virtual KoID colorModelId() const {
        return CMYKAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }
    virtual KoColorSpace* clone() const;
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;
};

class KoCmykU8ColorSpaceFactory : public KoLcmsColorSpaceFactory
{
public:
    KoCmykU8ColorSpaceFactory(): KoLcmsColorSpaceFactory(TYPE_CMYK5_8, icSigCmykData) {}
    virtual bool userVisible() const {
        return true;
    }
    virtual QString id() const {
        return "CMYK";
    }
    virtual QString name() const {
        return i18n("CMYK (8-bit integer/channel)");
    }
    virtual KoID colorModelId() const {
        return CMYKAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }
    virtual int referenceDepth() const {
        return 8;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new KoCmykU8ColorSpace(p->clone());
    }

    virtual QString defaultProfile() const {
        return "Adobe CMYK";
    }
};


#endif
