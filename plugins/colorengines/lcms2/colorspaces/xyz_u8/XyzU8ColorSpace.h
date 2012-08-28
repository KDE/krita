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

#ifndef KIS_XYZ_U8_COLORSPACE_H_
#define KIS_XYZ_U8_COLORSPACE_H_

#include <LcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

#include <KoColorModelStandardIds.h>

#define TYPE_XYZA_8 (COLORSPACE_SH(PT_XYZ)|CHANNELS_SH(3)|BYTES_SH(1)|EXTRA_SH(1))

class XyzU8ColorSpace : public LcmsColorSpace<KoXyzU8Traits>
{
public:

    XyzU8ColorSpace(const QString &name, KoColorProfile *p);

    virtual bool willDegrade(ColorSpaceIndependence independence) const;

    static QString colorSpaceId()
    {
        return QString("XYZA8");
    }

    virtual KoID colorModelId() const {
        return XYZAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }

    virtual KoColorSpace* clone() const;

    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;

    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;

};


class XyzU8ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:

    XyzU8ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_XYZA_8, cmsSigXYZData) {
    }

    virtual QString id() const {
        return XyzU8ColorSpace::colorSpaceId();
    }

    virtual QString name() const {
        return i18n("XYZ (8-bit integer/channel)");
    }

    virtual bool userVisible() const {
        return true;
    }

    virtual KoID colorModelId() const {
        return XYZAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Integer8BitsColorDepthID;
    }

    virtual int referenceDepth() const {
        return 8;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new XyzU8ColorSpace(name(), p->clone());
    }

    virtual QString defaultProfile() const {
        return "XYZ built-in - (lcms internal)";
    }
};


#endif
