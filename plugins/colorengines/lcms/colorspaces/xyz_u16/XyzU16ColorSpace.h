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

#ifndef KIS_XYZ_U16_COLORSPACE_H_
#define KIS_XYZ_U16_COLORSPACE_H_

#include <LcmsColorSpace.h>
#include <KoColorSpaceTraits.h>

#include <KoColorModelStandardIds.h>

typedef KoXyzTraits<quint16> XyzU16Traits;

class XyzU16ColorSpace : public LcmsColorSpace<XyzU16Traits>
{
public:
    XyzU16ColorSpace(KoColorProfile *p);
    virtual bool willDegrade(ColorSpaceIndependence independence) const;
    virtual KoID colorModelId() const {
        return XYZAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer16BitsColorDepthID;
    }
    virtual KoColorSpace* clone() const;
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;
};

#define TYPE_XYZA_16 (COLORSPACE_SH(PT_XYZ)|CHANNELS_SH(3)|BYTES_SH(2)|EXTRA_SH(1))

class XyzU16ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    XyzU16ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_XYZA_16, icSigXYZData) {
    }
    virtual QString id() const {
        return "XYZA16";
    }
    virtual QString name() const {
        return i18n("XYZ (16-bit integer/channel)");
    }
    virtual bool userVisible() const {
        return true;
    }

    virtual KoID colorModelId() const {
        return XYZAColorModelID;
    }
    virtual KoID colorDepthId() const {
        return Integer16BitsColorDepthID;
    }
    virtual int referenceDepth() const {
        return 16;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new XyzU16ColorSpace(p->clone());
    }

    virtual QString defaultProfile() const {
        return "XYZ built-in - (lcms internal)";
    }
};


#endif
