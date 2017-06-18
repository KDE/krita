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

#ifndef KORGBU16COLORSPACE_H_
#define KORGBU16COLORSPACE_H_

#include "LcmsColorSpace.h"
#include "KoColorModelStandardIds.h"

struct KoBgrU16Traits;

class RgbU16ColorSpace : public LcmsColorSpace<KoBgrU16Traits>
{
public:
    RgbU16ColorSpace(const QString &name, KoColorProfile *p);

    bool willDegrade(ColorSpaceIndependence independence) const override;

    KoID colorModelId() const override
    {
        return RGBAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Integer16BitsColorDepthID;
    }

    virtual KoColorSpace *clone() const;

    void colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const override;

    void colorFromXML(quint8* pixel, const QDomElement& elt) const override;
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;

    static QString colorSpaceId()
    {
        return QString("RGBA16");
    }

};

class RgbU16ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    RgbU16ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_BGRA_16, cmsSigRgbData)
    {
    }
    QString id() const override
    {
        return RgbU16ColorSpace::colorSpaceId();
    }
    QString name() const override
    {
        return QString("%1 (%2)").arg(RGBAColorModelID.name()).arg(Integer16BitsColorDepthID.name());
    }

    bool userVisible() const override
    {
        return true;
    }
    KoID colorModelId() const override
    {
        return RGBAColorModelID;
    }
    KoID colorDepthId() const override
    {
        return Integer16BitsColorDepthID;
    }
    int referenceDepth() const override
    {
        return 16;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new RgbU16ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "sRGB-elle-V2-g10.icc";//this is a linear space, because 16bit is enough to only enjoy advantages of linear space
    }
};

#endif
