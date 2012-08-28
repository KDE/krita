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

#ifndef LabF32ColorSpace_H_
#define LabF32ColorSpace_H_

#include "LcmsColorSpace.h"
#include "KoColorSpaceTraits.h"
#include "KoColorModelStandardIds.h"

// XXX: implement normalizedChannelValues?

class LabF32ColorSpace : public LcmsColorSpace<KoLabF32Traits>
{
public:
    LabF32ColorSpace (const QString &name, KoColorProfile *p);

    virtual bool willDegrade(ColorSpaceIndependence independence) const;

    static QString colorSpaceId()
    {
        return QString("LABAF32");
    }

    virtual KoID colorModelId() const {
        return LABAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Float32BitsColorDepthID;
    }

    virtual KoColorSpace* clone() const;

    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;

    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;

    virtual bool hasHighDynamicRange() const {
        return true;
    }
};

class LabF32ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    LabF32ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_LabA_FLT, cmsSigLabData)
    {
    }

    virtual bool userVisible() const {
        return true;
    }

    virtual QString id() const {
        return LabF32ColorSpace::colorSpaceId();
    }

    virtual QString name() const {
        return i18n("L*a*b* (32-bit float/channel)");
    }

    virtual KoID colorModelId() const {
        return LABAColorModelID;
    }

    virtual KoID colorDepthId() const {
        return Float32BitsColorDepthID;

    }
    virtual int referenceDepth() const {
        return 32;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const {
        return new LabF32ColorSpace(name(), p->clone());
    }

    virtual QString defaultProfile() const {
        return "Lab identity built-in";
    }

    virtual bool isHdr() const
    {
        return true;
    }
};


#endif
