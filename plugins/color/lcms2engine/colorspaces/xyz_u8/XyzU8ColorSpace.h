/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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

#ifndef KIS_XYZ_U8_COLORSPACE_H_
#define KIS_XYZ_U8_COLORSPACE_H_

#include <LcmsColorSpace.h>

#include <KoColorModelStandardIds.h>

#define TYPE_XYZA_8 (COLORSPACE_SH(PT_XYZ)|CHANNELS_SH(3)|BYTES_SH(1)|EXTRA_SH(1))

struct KoXyzU8Traits;

class XyzU8ColorSpace : public LcmsColorSpace<KoXyzU8Traits>
{
public:

    XyzU8ColorSpace(const QString &name, KoColorProfile *p);

    bool willDegrade(ColorSpaceIndependence independence) const override;

    static QString colorSpaceId()
    {
        return QString("XYZA8");
    }

    KoID colorModelId() const override
    {
        return XYZAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Integer8BitsColorDepthID;
    }

    virtual KoColorSpace *clone() const;

    void colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const override;

    void colorFromXML(quint8* pixel, const QDomElement& elt) const override;
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;

};

class XyzU8ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:

    XyzU8ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_XYZA_8, cmsSigXYZData)
    {
    }

    QString id() const override
    {
        return XyzU8ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(XYZAColorModelID.name()).arg(Integer8BitsColorDepthID.name());
    }

    bool userVisible() const override
    {
        return true;
    }

    KoID colorModelId() const override
    {
        return XYZAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Integer8BitsColorDepthID;
    }

    int referenceDepth() const override
    {
        return 8;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new XyzU8ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "XYZ identity built-in";
    }
};

#endif
