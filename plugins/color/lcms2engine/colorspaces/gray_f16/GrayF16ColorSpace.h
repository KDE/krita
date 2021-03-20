/*
 *  SPDX-FileCopyrightText: 2004-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        return "GRAYAF16";
    }

    bool hasHighDynamicRange() const override
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

    QString id() const override
    {
        return GrayF16ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(GrayAColorModelID.name()).arg(Float16BitsColorDepthID.name());
    }

    KoID colorModelId() const override
    {
        return GrayAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Float16BitsColorDepthID;
    }

    int referenceDepth() const override
    {
        return 16;
    }

    bool userVisible() const override
    {
        return true;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new GrayF16ColorSpace(name(), p->clone());
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
