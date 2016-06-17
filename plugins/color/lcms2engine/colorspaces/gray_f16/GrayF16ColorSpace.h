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
#ifndef COLORSPACE_GRAYSCALE_F16_H_
#define COLORSPACE_GRAYSCALE_F16_H_

#include <klocalizedstring.h>
#include <KoColorModelStandardIds.h>
#include "LcmsColorSpace.h"

#define TYPE_GRAYA_HALF_FLT         (FLOAT_SH(1)|COLORSPACE_SH(PT_GRAY)|EXTRA_SH(1)|CHANNELS_SH(1)|BYTES_SH(2))

struct KoGrayF16Traits;

class GrayF16ColorSpace : public LcmsColorSpace<KoGrayF16Traits>
{
public:
    GrayF16ColorSpace(const QString &name, KoColorProfile *p);

    virtual bool willDegrade(ColorSpaceIndependence) const
    {
        return false;
    }

    virtual KoID colorModelId() const
    {
        return GrayAColorModelID;
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
        return "GRAYAF16";
    }

    virtual bool hasHighDynamicRange() const
    {
        return true;
    }
};

class GrayF16ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    GrayF16ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_GRAYA_HALF_FLT, cmsSigGrayData)
    {
    }

    virtual QString id() const
    {
        return GrayF16ColorSpace::colorSpaceId();
    }

    virtual QString name() const
    {
        return i18n("Grayscale/Alpha (16-bit float/channel)");
    }

    virtual KoID colorModelId() const
    {
        return GrayAColorModelID;
    }

    virtual KoID colorDepthId() const
    {
        return Float16BitsColorDepthID;
    }

    virtual int referenceDepth() const
    {
        return 16;
    }

    virtual bool userVisible() const
    {
        return true;
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *p) const
    {
        return new GrayF16ColorSpace(name(), p->clone());
    }

    virtual QString defaultProfile() const
    {
        return "gray built-in";
    }

    virtual bool isHdr() const
    {
        return true;
    }
};

#endif // KIS_STRATEGY_COLORSPACE_GRAYSCALE_H_
