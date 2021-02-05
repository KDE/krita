/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "LabF32ColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include "../compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

LabF32ColorSpace::LabF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoLabF32Traits>(colorSpaceId(), name, TYPE_LabA_FLT, cmsSigLabData, p)
{
    const IccColorProfile *icc_p = dynamic_cast<const IccColorProfile *>(p);
    Q_ASSERT(icc_p);
    QVector<KoChannelInfo::DoubleRange> uiRanges(icc_p->getFloatUIMinMax());
    Q_ASSERT(uiRanges.size() == 3);

    addChannel(new KoChannelInfo(i18nc("Lightness value in Lab color model", "Lightness"), 0 * sizeof(float), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), QColor(100, 100, 100), uiRanges[0]));
    addChannel(new KoChannelInfo(i18n("a*"),        1 * sizeof(float), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), QColor(150, 150, 150), uiRanges[1]));
    addChannel(new KoChannelInfo(i18n("b*"),        2 * sizeof(float), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), QColor(200, 200, 200), uiRanges[2]));
    addChannel(new KoChannelInfo(i18n("Alpha"),     3 * sizeof(float), 3, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoLabF32Traits>(this);

    dbgPlugins << "La*b* (float) channel bounds for: " << icc_p->name();
    dbgPlugins << "L: " << uiRanges[0].minVal << uiRanges[0].maxVal;
    dbgPlugins << "a: " << uiRanges[1].minVal << uiRanges[1].maxVal;
    dbgPlugins << "b: " << uiRanges[2].minVal << uiRanges[2].maxVal;
}

bool LabF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *LabF32ColorSpace::clone() const
{
    return new LabF32ColorSpace(name(), profile()->clone());
}

void LabF32ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoLabF32Traits::Pixel *p = reinterpret_cast<const KoLabF32Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("Lab");

    // we need 0-100, -128-+127

    labElt.setAttribute("L", KisDomUtils::toString(KoColorSpaceMaths< KoLabF32Traits::channels_type, qreal>::scaleToA(p->L)));

    labElt.setAttribute("a", KisDomUtils::toString(KoColorSpaceMaths< KoLabF32Traits::channels_type, qreal>::scaleToA(p->a)));

    labElt.setAttribute("b", KisDomUtils::toString(KoColorSpaceMaths< KoLabF32Traits::channels_type, qreal>::scaleToA(p->b)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void LabF32ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoLabF32Traits::Pixel *p = reinterpret_cast<KoLabF32Traits::Pixel *>(pixel);
    p->L = KoColorSpaceMaths< qreal, KoLabF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("L")));
    p->a = KoColorSpaceMaths< qreal, KoLabF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("a")));
    p->b = KoColorSpaceMaths< qreal, KoLabF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("b")));
    p->alpha = 1.0;
}

void LabF32ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> LabF32ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}
void LabF32ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *u=channelValues[1];
    *v=channelValues[2];
}

QVector <double> LabF32ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*u;
    channelValues[2]=*v;
    channelValues[3]=1.0;
    return channelValues;
}

quint8 LabF32ColorSpace::scaleToU8(const quint8 *srcPixel, qint32 channelIndex) const
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

void LabF32ColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const
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

void LabF32ColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const
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
