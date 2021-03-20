/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LabU8ColorSpace_H_
#define LabU8ColorSpace_H_

#include "LcmsColorSpace.h"
#include "KoColorModelStandardIds.h"

#define TYPE_LABA_8 (COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(1) | EXTRA_SH(1))

struct KoLabU8Traits;

class LabU8ColorSpace : public LcmsColorSpace<KoLabU8Traits>
{
public:
    LabU8ColorSpace(const QString &name, KoColorProfile *p);
    bool willDegrade(ColorSpaceIndependence independence) const override;

    static QString colorSpaceId()
    {
        return QString("LABAU8");
    }

    KoID colorModelId() const override
    {
        return LABAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Integer8BitsColorDepthID;
    }

    virtual KoColorSpace* clone() const;
    void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const override;
    void colorFromXML(quint8* pixel, const QDomElement& elt) const override;
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;
    quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const override;
    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const override;
    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const override;
};

class LabU8ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    LabU8ColorSpaceFactory() : LcmsColorSpaceFactory(TYPE_LABA_8, cmsSigLabData) {}

    bool userVisible() const override
    {
        return true;
    }

    QString id() const override
    {
        return LabU8ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(LABAColorModelID.name()).arg(Integer8BitsColorDepthID.name());
    }

    KoID colorModelId() const override
    {
        return LABAColorModelID;
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
        return new LabU8ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "Lab identity built-in";
    }
};

#endif
