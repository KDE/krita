/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KO_STRATEGY_COLORSPACE_RGB_H_
#define KO_STRATEGY_COLORSPACE_RGB_H_

#include <klocalizedstring.h>
#include <LcmsColorSpace.h>
#include "KoColorModelStandardIds.h"

struct KoBgrU8Traits;

class RgbU8ColorSpace : public LcmsColorSpace<KoBgrU8Traits>
{

public:

    RgbU8ColorSpace(const QString &name, KoColorProfile *p);

    bool willDegrade(ColorSpaceIndependence) const override
    {
        return false;
    }

    KoID colorModelId() const override
    {
        return RGBAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Integer8BitsColorDepthID;
    }

    virtual KoColorSpace *clone() const;

    void colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const override;

    void colorFromXML(quint8 *pixel, const QDomElement &elt) const override;

    quint8 intensity8(const quint8 * src) const override;
    
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;

    static QString colorSpaceId()
    {
        return QString("RGBA");
    }

    void fillGrayBrushWithColorAndLightnessOverlay(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const override;
    void fillGrayBrushWithColorAndLightnessWithStrength(quint8* dst, const QRgb* brush, quint8* brushColor, qreal strength, qint32 nPixels) const override;
};

class RgbU8ColorSpaceFactory : public LcmsColorSpaceFactory
{

public:

    RgbU8ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_BGRA_8, cmsSigRgbData) {}

    bool userVisible() const override
    {
        return true;
    }

    QString id() const override
    {
        return RgbU8ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(RGBAColorModelID.name()).arg(Integer8BitsColorDepthID.name());
    }

    KoID colorModelId() const override
    {
        return RGBAColorModelID;
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
        return new RgbU8ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "sRGB-elle-V2-srgbtrc.icc";
    }
};

#endif // KO_STRATEGY_COLORSPACE_RGB_H_
