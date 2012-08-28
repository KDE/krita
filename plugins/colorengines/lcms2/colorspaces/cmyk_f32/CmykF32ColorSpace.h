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

#ifndef KIS_COLORSPACE_CMYK_F32_H_
#define KIS_COLORSPACE_CMYK_F32_H_

#include <LcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

#include "KoColorModelStandardIds.h"

#define TYPE_CMYKA_FLT        (FLOAT_SH(1)|COLORSPACE_SH(PT_CMYK)|EXTRA_SH(1)|CHANNELS_SH(4)|BYTES_SH(4))

class CmykF32ColorSpace : public LcmsColorSpace<KoCmykF32Traits>
{
public:
    CmykF32ColorSpace(const QString &name, KoColorProfile *p);

    virtual bool willDegrade(ColorSpaceIndependence independence) const;

    virtual KoID colorModelId() const {
        return CMYKAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Float32BitsColorDepthID;
    }

    virtual KoColorSpace* clone() const;

    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;

    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;

    static QString colorSpaceId()
    {
        return "CMYKAF32";
    }

    virtual bool hasHighDynamicRange() const {
        return true;
    }
};

class CmykF32ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:

    CmykF32ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_CMYKA_FLT, cmsSigCmykData)
    {
    }

    virtual bool userVisible() const {
        return true;
    }

    virtual QString id() const {
        return CmykF32ColorSpace::colorSpaceId();
    }

    virtual QString name() const {
        return i18n("CMYK (32 bits floating/channel)");
    }

    virtual KoID colorModelId() const {
        return CMYKAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Float32BitsColorDepthID;
    }

    virtual int referenceDepth() const {
        return 32;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new CmykF32ColorSpace(name(), p->clone());
    }

    virtual QString defaultProfile() const {
        return "Offset printing, according to ISO/DIS 12647-2:2004, OFCOM, paper type 1 or 2 = coated art, 115 g/m2, screen ruling 60 cm-1, positive-acting plates";
    }

    virtual bool isHdr() const
    {
        return true;
    }
};


#endif
