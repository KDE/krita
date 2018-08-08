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

#include <klocalizedstring.h>
#include <KoColorModelStandardIds.h>
#include "LcmsColorSpace.h"

#define TYPE_GRAYA_FLT         (FLOAT_SH(1)|COLORSPACE_SH(PT_GRAY)|EXTRA_SH(1)|CHANNELS_SH(1)|BYTES_SH(4))

struct KoGrayF32Traits;

class GrayF32ColorSpace : public LcmsColorSpace<KoGrayF32Traits>
{
public:
    GrayF32ColorSpace(const QString &name, KoColorProfile *p);

    bool willDegrade(ColorSpaceIndependence) const override
    {
        return false;
    }

    KoID colorModelId() const override
    {
        return GrayAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Float32BitsColorDepthID;
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
        return "GRAYAF32";
    }

    bool hasHighDynamicRange() const override
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

    QString id() const override
    {
        return GrayF32ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(GrayAColorModelID.name()).arg(Float32BitsColorDepthID.name());
    }

    KoID colorModelId() const override
    {
        return GrayAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Float32BitsColorDepthID;
    }

    int referenceDepth() const override
    {
        return 32;
    }

    bool userVisible() const override
    {
        return true;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new GrayF32ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "Gray-D50-elle-V2-g10.icc";
    }

    bool isHdr() const override
    {
        return true;
    }
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
