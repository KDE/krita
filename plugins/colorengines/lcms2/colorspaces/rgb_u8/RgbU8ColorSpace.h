/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KO_STRATEGY_COLORSPACE_RGB_H_
#define KO_STRATEGY_COLORSPACE_RGB_H_

#include <klocale.h>
#include <LcmsColorSpace.h>
#include <KoColorSpaceTraits.h>
#include "KoColorModelStandardIds.h"

class RgbU8ColorSpace : public LcmsColorSpace<KoRgbU8Traits>
{

public:

    RgbU8ColorSpace(KoColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence) const {
        return false;
    }
    virtual KoColorTransformation* createInvertTransformation() const;
    virtual KoID colorModelId() const {
        return RGBAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }
    virtual KoColorSpace* clone() const;
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;
    virtual quint8 intensity8(const quint8 * src) const;

    /**
     * The ID that identifies this colorspace. Pass this as the colorSpaceId parameter
     * to the KoColorSpaceRegistry::colorSpace() functions to obtain this colorspace.
     * This is the value that the member function id() returns.
     */
    static QString colorSpaceId();
};

class RgbU8ColorSpaceFactory : public LcmsColorSpaceFactory
{

public:

    RgbU8ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_BGRA_8, cmsSigRgbData) {}
    virtual bool userVisible() const {
        return true;
    }
    virtual QString id() const {
        return RgbU8ColorSpace::colorSpaceId();
    }
    virtual QString name() const {
        return i18n("RGB (8-bit integer/channel)");
    }
    virtual KoID colorModelId() const {
        return RGBAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }
    virtual int referenceDepth() const {
        return 8;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile * p) const {
        return new RgbU8ColorSpace(p->clone());
    }

    virtual QString defaultProfile() const {
        return "sRGB built-in - (lcms internal)";
    }
};

#endif // KO_STRATEGY_COLORSPACE_RGB_H_
