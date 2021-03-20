/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "LabColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include "../compositeops/KoCompositeOps.h"
#include "dithering/KisLabDitherOpFactory.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

LabU16ColorSpace::LabU16ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoLabU16Traits>(colorSpaceId(), name, TYPE_LABA_16, cmsSigLabData, p)
{
    addChannel(new KoChannelInfo(i18nc("Lightness value in Lab color model", "Lightness"), 0 * sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(100, 100, 100)));
    addChannel(new KoChannelInfo(i18n("a*"),        1 * sizeof(quint16), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(150, 150, 150)));
    addChannel(new KoChannelInfo(i18n("b*"),        2 * sizeof(quint16), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(200, 200, 200)));
    addChannel(new KoChannelInfo(i18n("Alpha"),     3 * sizeof(quint16), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    init();

    addStandardCompositeOps<KoLabU16Traits>(this);
    addStandardDitherOps<KoLabU16Traits>(this);
}

bool LabU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *LabU16ColorSpace::clone() const
{
    return new LabU16ColorSpace(name(), profile()->clone());
}

void LabU16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoLabU16Traits::Pixel *p = reinterpret_cast<const KoLabU16Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("Lab");

    qreal a, b;
    KoLabU16Traits::channels_type halfValue = KoLabColorSpaceMathsTraits<KoLabU16Traits::channels_type>::halfValueAB;

    if (p->a <= halfValue) {
        a = double(double(halfValue - p->a) / halfValue);
        a = a * -128.0;
    } else {
        a = double(double(p->a - halfValue) / halfValue);
        a = 127.0 * a;
    }

    if (p->b <= halfValue) {
        b = double(double(halfValue - p->b) / halfValue);
        b = b * -128.0;
    } else {
        b = double(double(p->b - halfValue) / halfValue);
        b = 127.0 * b;
    }

    labElt.setAttribute("L", KisDomUtils::toString(KoColorSpaceMaths<KoLabU16Traits::channels_type, qreal>::scaleToA(p->L)*100.0f));
    labElt.setAttribute("a", KisDomUtils::toString(a));
    labElt.setAttribute("b", KisDomUtils::toString(b));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void LabU16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoLabU16Traits::Pixel *p = reinterpret_cast<KoLabU16Traits::Pixel *>(pixel);

    double a = KisDomUtils::toDouble(elt.attribute("a"));
    double b = KisDomUtils::toDouble(elt.attribute("b"));

    // L will go from 0 to 100

    p->L = KoColorSpaceMaths<qreal, KoLabU16Traits::channels_type>::scaleToA(KisDomUtils::toDouble(elt.attribute("L"))*0.01f);
    KoLabU16Traits::channels_type halfValue = KoLabColorSpaceMathsTraits<KoLabU16Traits::channels_type>::halfValueAB;

    // a goes from  -128 to 127
    if(a <= 0) {
        a = (a/-128.0);
        p->a = halfValue - (a * halfValue);
    } else {
        a = fabs(a/127.0);
        p->a = (a * halfValue) + halfValue;
    }

    if(b <= 0) {
        b = (b/-128.0);
        p->b = halfValue - (b * halfValue);
    } else {
        b = fabs(b/127.0);
        p->b = (b * halfValue) + halfValue;
    }

    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}
void LabU16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> LabU16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void LabU16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *u=channelValues[1];
    *v=channelValues[2];
}

QVector <double> LabU16ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*u;
    channelValues[2]=*v;
    channelValues[3]=1.0;
    return channelValues;
}

quint8 LabU16ColorSpace::scaleToU8(const quint8 *srcPixel, qint32 channelIndex) const
{
    typename ColorSpaceTraits::channels_type c = ColorSpaceTraits::nativeArray(srcPixel)[channelIndex];
    qreal b = 0;
    switch (channelIndex) {
    case ColorSpaceTraits::L_pos:
        b = ((qreal)c) / ColorSpaceTraits::math_trait::unitValueL;
        break;
    case ColorSpaceTraits::a_pos:
    case ColorSpaceTraits::b_pos:
        if (c <= ColorSpaceTraits::math_trait::halfValueAB) {
            b = ((qreal)c - ColorSpaceTraits::math_trait::zeroValueAB) / (2.0 * (ColorSpaceTraits::math_trait::halfValueAB - ColorSpaceTraits::math_trait::zeroValueAB));
        } else {
            b = 0.5 + ((qreal)c - ColorSpaceTraits::math_trait::halfValueAB) / (2.0 * (ColorSpaceTraits::math_trait::unitValueAB - ColorSpaceTraits::math_trait::halfValueAB));
        }
        break;
    default:
        b = ((qreal)c) / ColorSpaceTraits::math_trait::unitValue;
        break;
    }

    return KoColorSpaceMaths<qreal, quint8>::scaleToA(b);
}

void LabU16ColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const
{
    for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
        for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
            KoChannelInfo *channel = this->channels().at(channelIndex);
            qint32 channelSize = channel->size();
            if (channel->channelType() == KoChannelInfo::COLOR) {
                if (channelIndex == ColorSpaceTraits::L_pos) {
                    ColorSpaceTraits::channels_type c = ColorSpaceTraits::parent::nativeArray((src + (pixelIndex * ColorSpaceTraits::pixelSize)))[selectedChannelIndex];
                    switch (selectedChannelIndex) {
                    case ColorSpaceTraits::L_pos:
                        break;
                    case ColorSpaceTraits::a_pos:
                    case ColorSpaceTraits::b_pos:
                        if (c <= ColorSpaceTraits::math_trait::halfValueAB) {
                            c = ColorSpaceTraits::math_trait::unitValueL * (((qreal)c - ColorSpaceTraits::math_trait::zeroValueAB) / (2.0 * (ColorSpaceTraits::math_trait::halfValueAB - ColorSpaceTraits::math_trait::zeroValueAB)));
                        } else {
                            c = ColorSpaceTraits::math_trait::unitValueL * (0.5 + ((qreal)c - ColorSpaceTraits::math_trait::halfValueAB) / (2.0 * (ColorSpaceTraits::math_trait::unitValueAB - ColorSpaceTraits::math_trait::halfValueAB)));
                        }
                        break;
                    // As per KoChannelInfo alpha channels are [0..1]
                    default:
                        c = ColorSpaceTraits::math_trait::unitValueL * (qreal)c / ColorSpaceTraits::math_trait::unitValue;
                        break;
                    }
                    ColorSpaceTraits::parent::nativeArray(dst + (pixelIndex * ColorSpaceTraits::pixelSize))[channelIndex] = c;
                } else {
                    ColorSpaceTraits::parent::nativeArray(dst + (pixelIndex * ColorSpaceTraits::pixelSize))[channelIndex] = ColorSpaceTraits::math_trait::halfValueAB;
                }
            } else if (channel->channelType() == KoChannelInfo::ALPHA) {
                memcpy(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), channelSize);
            }
        }
    }
}

void LabU16ColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const
{
    for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
        for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
            KoChannelInfo *channel = this->channels().at(channelIndex);
            qint32 channelSize = channel->size();
            if (selectedChannels.testBit(channelIndex)) {
                memcpy(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), channelSize);
            } else {
                ColorSpaceTraits::channels_type v;
                switch (channelIndex) {
                case ColorSpaceTraits::L_pos:
                    v = ColorSpaceTraits::math_trait::halfValueL;
                    break;
                case ColorSpaceTraits::a_pos:
                case ColorSpaceTraits::b_pos:
                    v = ColorSpaceTraits::math_trait::halfValueAB;
                    break;
                default:
                    v = ColorSpaceTraits::math_trait::zeroValue;
                    break;
                }
                reinterpret_cast<ColorSpaceTraits::channels_type *>(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize))[0] = v;
            }
        }
    }
}
