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

#ifndef KIS_XYZ_F16_COLORSPACE_H_
#define KIS_XYZ_F16_COLORSPACE_H_

#include <LcmsColorSpace.h>

#define TYPE_XYZA_HALF_FLT (FLOAT_SH(1)|COLORSPACE_SH(PT_XYZ)|EXTRA_SH(1)|CHANNELS_SH(3)|BYTES_SH(2))

#include <KoColorModelStandardIds.h>

struct KoXyzF16Traits;

class XyzF16ColorSpace : public LcmsColorSpace<KoXyzF16Traits>
{
public:

    XyzF16ColorSpace(const QString &name, KoColorProfile *p);

    bool willDegrade(ColorSpaceIndependence independence) const override;

    KoID colorModelId() const override
    {
        return XYZAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Float16BitsColorDepthID;
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
        return QString("XYZAF16");
    }

    bool hasHighDynamicRange() const override
    {
        return true;
    }
};

class XyzF16ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:

    XyzF16ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_XYZA_HALF_FLT, cmsSigXYZData)
    {
    }

    QString id() const override
    {
        return XyzF16ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(XYZAColorModelID.name()).arg(Float16BitsColorDepthID.name());
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
        return Float16BitsColorDepthID;
    }

    int referenceDepth() const override
    {
        return 16;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new XyzF16ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "XYZ identity built-in";
    }

    bool isHdr() const override
    {
        return true;
    }
};

#endif
