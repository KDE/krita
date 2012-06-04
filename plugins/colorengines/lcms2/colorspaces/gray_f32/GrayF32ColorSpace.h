/*
 *  Copyright (c) 2004-2006 Cyrille Berger <cberger@cberger.net>
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
#ifndef COLORSPACE_GRAYSCALE_F32_H_
#define COLORSPACE_GRAYSCALE_F32_H_

#include <klocale.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>
#include "LcmsColorSpace.h"

#define TYPE_GRAYA_FLT         (FLOAT_SH(1)|COLORSPACE_SH(PT_GRAY)|EXTRA_SH(1)|CHANNELS_SH(1)|BYTES_SH(4))

class GrayF32ColorSpace : public LcmsColorSpace<KoGrayF32Traits>
{
public:
    GrayF32ColorSpace(const QString &name, KoColorProfile *p);

    virtual bool willDegrade(ColorSpaceIndependence) const {
        return false;
    }

    virtual KoID colorModelId() const {
        return GrayAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Float32BitsColorDepthID;
    }

    virtual KoColorSpace* clone() const;

    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;

    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;

    static QString colorSpaceId()
    {
        return "GRAYAF32";
    }

    virtual bool hasHighDynamicRange() const 
    {
        return true;
    }
};

class GrayF32ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    GrayF32ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_GRAYA_FLT, cmsSigGrayData)
    {
    }

    virtual QString id() const {
        return GrayF32ColorSpace::colorSpaceId();
    }

    virtual QString name() const {
        return i18n("Grayscale/Alpha (32-bit float/channel)");
    }

    virtual KoID colorModelId() const {
        return GrayAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Float32BitsColorDepthID;
    }

    virtual int referenceDepth() const {
        return 32;
    }

    virtual bool userVisible() const {
        return true;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new GrayF32ColorSpace(name(), p->clone());
    }

    virtual QString defaultProfile() const {
        return "gray built-in";
    }

    virtual bool isHdr() const
    {
        return true;
    }
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
