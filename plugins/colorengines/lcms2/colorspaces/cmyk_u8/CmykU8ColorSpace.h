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

#include <LcmsColorSpace.h>
#include <KoColorSpaceTraits.h>
#include "KoColorModelStandardIds.h"

typedef KoCmykTraits<quint8> CmykU8Traits;

class CmykU8ColorSpace : public LcmsColorSpace<CmykU8Traits>
{
public:
    CmykU8ColorSpace(KoColorProfile *p);
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

class CmykU8ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    CmykU8ColorSpaceFactory(): LcmsColorSpaceFactory(TYPE_CMYK5_8, cmsSigCmykData) {}
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
        return new CmykU8ColorSpace(p->clone());
    }

    virtual QString defaultProfile() const {
        return "Offset printing, according to ISO/DIS 12647-2:2004, OFCOM, paper type 1 or 2 = coated art, 115 g/m2, screen ruling 60 cm-1, positive-acting plates";
    }
};


#endif
