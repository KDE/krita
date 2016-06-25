/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef KORGBF16COLORSPACE_H_
#define KORGBF16COLORSPACE_H_

#include "LcmsColorSpace.h"
#include "KoColorModelStandardIds.h"

struct KoRgbF16Traits;

class RgbF16ColorSpace : public LcmsColorSpace<KoRgbF16Traits>
{
public:
    RgbF16ColorSpace(const QString &name, KoColorProfile *p);

    virtual bool willDegrade(ColorSpaceIndependence independence) const;

    virtual KoID colorModelId() const
    {
        return RGBAColorModelID;
    }

    virtual KoID colorDepthId() const
    {
        return Float16BitsColorDepthID;
    }

    virtual KoColorSpace *clone() const;

    virtual void colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const;

    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;
    virtual void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const;
    virtual QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const;
    virtual void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const;
    virtual QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const;

    static QString colorSpaceId()
    {
        return QString("RGBAF16");
    }

    virtual bool hasHighDynamicRange() const
    {
        return true;
    }
};

class RgbF16ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    RgbF16ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_RGBA_HALF_FLT, cmsSigRgbData)
    {
    }

    virtual QString id() const
    {
        return RgbF16ColorSpace::colorSpaceId();
    }

    virtual QString name() const
    {
        return i18n("RGBA (16-bit floating/channel)");
    }

    virtual bool userVisible() const
    {
        return true;
    }

    virtual KoID colorModelId() const
    {
        return RGBAColorModelID;
    }

    virtual KoID colorDepthId() const
    {
        return Float16BitsColorDepthID;
    }

    virtual int referenceDepth() const
    {
        return 16;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const
    {
        return new RgbF16ColorSpace(name(), p->clone());
    }

    virtual QString defaultProfile() const
    {
        return "sRGB-elle-V2-g10.icc";
    }

    virtual bool isHdr() const
    {
        return true;
    }

};

#endif
