/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "RgbF16ColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>
#include <KoColorConversions.h>
#include "compositeops/KoCompositeOps.h"

#include "compositeops/RgbCompositeOpIn.h"
#include "compositeops/RgbCompositeOpOut.h"
#include "compositeops/RgbCompositeOpBumpmap.h"
#include "dithering/KisRgbDitherOpFactory.h"
#include <kis_dom_utils.h>
#include <KoColorSpacePreserveLightnessUtils.h>

RgbF16ColorSpace::RgbF16ColorSpace(const QString &name, KoColorProfile *p) :
    LcmsColorSpace<KoRgbF16Traits>(colorSpaceId(), name, TYPE_RGBA_HALF_FLT, cmsSigRgbData, p)
{
    addChannel(new KoChannelInfo(i18n("Red"), 0 * sizeof(half), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, QColor(255, 0, 0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1 * sizeof(half), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, QColor(0, 255, 0)));
    addChannel(new KoChannelInfo(i18n("Blue"), 2 * sizeof(half), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, QColor(0, 0, 255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3 * sizeof(half), 3, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT16, 2));

    init();

    addStandardCompositeOps<KoRgbF16Traits>(this);
    addStandardDitherOps<KoRgbF16Traits>(this);

    addCompositeOp(new RgbCompositeOpIn<KoRgbF16Traits>(this));
    addCompositeOp(new RgbCompositeOpOut<KoRgbF16Traits>(this));
    addCompositeOp(new RgbCompositeOpBumpmap<KoRgbF16Traits>(this));
}

bool RgbF16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *RgbF16ColorSpace::clone() const
{
    return new RgbF16ColorSpace(name(), profile()->clone());
}

void RgbF16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoRgbF16Traits::Pixel *p = reinterpret_cast<const KoRgbF16Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("RGB");
    labElt.setAttribute("r", KisDomUtils::toString(KoColorSpaceMaths< KoRgbF16Traits::channels_type, qreal>::scaleToA(p->red)));
    labElt.setAttribute("g", KisDomUtils::toString(KoColorSpaceMaths< KoRgbF16Traits::channels_type, qreal>::scaleToA(p->green)));
    labElt.setAttribute("b", KisDomUtils::toString(KoColorSpaceMaths< KoRgbF16Traits::channels_type, qreal>::scaleToA(p->blue)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void RgbF16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoRgbF16Traits::Pixel *p = reinterpret_cast<KoRgbF16Traits::Pixel *>(pixel);
    p->red = KoColorSpaceMaths< qreal, KoRgbF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("r")));
    p->green = KoColorSpaceMaths< qreal, KoRgbF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("g")));
    p->blue = KoColorSpaceMaths< qreal, KoRgbF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("b")));
    p->alpha = 1.0;
}

void RgbF16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    RGBToHSY(channelValues[0],channelValues[1],channelValues[2], hue, sat, luma, lumaCoefficients()[0], lumaCoefficients()[1], lumaCoefficients()[2]);
}

QVector <double> RgbF16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    HSYToRGB(*hue, *sat, *luma, &channelValues[0],&channelValues[1],&channelValues[2], lumaCoefficients()[0], lumaCoefficients()[1], lumaCoefficients()[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void RgbF16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{

    
    RGBToYUV(channelValues[0],channelValues[1],channelValues[2], y, u, v, lumaCoefficients()[0], lumaCoefficients()[1], lumaCoefficients()[2]);
}

QVector <double> RgbF16ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);

    YUVToRGB(*y, *u, *v, &channelValues[0],&channelValues[1],&channelValues[2], lumaCoefficients()[0], lumaCoefficients()[1], lumaCoefficients()[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void RgbF16ColorSpace::fillGrayBrushWithColorAndLightnessOverlay(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const
{
    fillGrayBrushWithColorPreserveLightnessRGB<KoRgbF16Traits>(dst, brush, brushColor, 1.0, nPixels);
}

void RgbF16ColorSpace::fillGrayBrushWithColorAndLightnessWithStrength(quint8* dst, const QRgb* brush, quint8* brushColor, qreal strength, qint32 nPixels) const
{
    fillGrayBrushWithColorPreserveLightnessRGB<KoRgbF16Traits>(dst, brush, brushColor, strength, nPixels);
}

void RgbF16ColorSpace::modulateLightnessByGrayBrush(quint8 *dst, const QRgb *brush, qreal strength, qint32 nPixels) const
{
   modulateLightnessByGrayBrushRGB<KoRgbF16Traits>(dst, brush, strength, nPixels);
}
