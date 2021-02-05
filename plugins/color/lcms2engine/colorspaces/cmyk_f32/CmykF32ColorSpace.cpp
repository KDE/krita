/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "CmykF32ColorSpace.h"

#include <QDomElement>
#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

CmykF32ColorSpace::CmykF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoCmykF32Traits>(colorSpaceId(), name,  TYPE_CMYKA_FLT, cmsSigCmykData, p)
{
    const IccColorProfile *icc_p = dynamic_cast<const IccColorProfile *>(p);
    Q_ASSERT(icc_p);
    QVector<KoChannelInfo::DoubleRange> uiRanges(icc_p->getFloatUIMinMax());
    Q_ASSERT(uiRanges.size() == 4);

    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(float), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::cyan, uiRanges[0]));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(float), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::magenta, uiRanges[1]));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(float), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::yellow, uiRanges[2]));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(float), 3, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::black, uiRanges[3]));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(float), 4, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    dbgPlugins << "CMYK (float) profile bounds for: " << icc_p->name();
    dbgPlugins << "C: " << uiRanges[0].minVal << uiRanges[0].maxVal;
    dbgPlugins << "M: " << uiRanges[1].minVal << uiRanges[1].maxVal;
    dbgPlugins << "Y: " << uiRanges[2].minVal << uiRanges[2].maxVal;
    dbgPlugins << "K: " << uiRanges[3].minVal << uiRanges[3].maxVal;

    addStandardCompositeOps<KoCmykF32Traits>(this);
}

bool CmykF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *CmykF32ColorSpace::clone() const
{
    return new CmykF32ColorSpace(name(), profile()->clone());
}

void CmykF32ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoCmykF32Traits::Pixel *p = reinterpret_cast<const KoCmykF32Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("CMYK");

    // XML expects 0-1, we need 0-100
    // Get the bounds from the channels and adjust the calculations
    labElt.setAttribute("c", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA((1.f / this->channels()[0]->getUIUnitValue()) * (p->cyan - this->channels()[0]->getUIMin()))));
    labElt.setAttribute("m", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA((1.f / this->channels()[1]->getUIUnitValue()) * (p->magenta - this->channels()[1]->getUIMin()))));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA((1.f / this->channels()[2]->getUIUnitValue()) * (p->yellow - this->channels()[2]->getUIMin()))));
    labElt.setAttribute("k", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA((1.f / this->channels()[3]->getUIUnitValue()) * (p->black - this->channels()[3]->getUIMin()))));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykF32ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoCmykF32Traits::Pixel *p = reinterpret_cast<KoCmykF32Traits::Pixel *>(pixel);
    p->cyan = this->channels()[0]->getUIMin() + KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("c"))) * this->channels()[0]->getUIUnitValue();
    p->magenta = this->channels()[1]->getUIMin() + KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("m"))) * this->channels()[1]->getUIUnitValue();
    p->yellow = this->channels()[2]->getUIMin() + KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y"))) * this->channels()[2]->getUIUnitValue();
    p->black = this->channels()[3]->getUIMin() + KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("k"))) * this->channels()[3]->getUIUnitValue();
    p->alpha = 1.0;
}

void CmykF32ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    qreal c0 = channelValues[0];
    qreal c1 = channelValues[1];
    qreal c2 = channelValues[2];
    qreal c3 = channelValues[3];
    //we use HSI here because we can't linearise CMYK, and HSY doesn't work right with...
    CMYKToCMY(&c0, &c1, &c2, &c3);
    c0 = 1.0 - c0;
    c1 = 1.0 - c1;
    c2 = 1.0 - c2;
    RGBToHSI(c0, c1, c2, hue, sat, luma);
}

QVector <double> CmykF32ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(5);
    channelValues.fill(1.0);
    HSIToRGB(*hue, *sat, *luma, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[0] = qBound(0.0,1.0-channelValues[0],1.0);
    channelValues[1] = qBound(0.0,1.0-channelValues[1],1.0);
    channelValues[2] = qBound(0.0,1.0-channelValues[2],1.0);
    CMYToCMYK(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    return channelValues;
}

void CmykF32ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    qreal c0 = channelValues[0];
    qreal c1 = channelValues[1];
    qreal c2 = channelValues[2];
    qreal c3 = channelValues[3];
    CMYKToCMY(&c0, &c1, &c2, &c3);
    c0 = 1.0 - c0;
    c1 = 1.0 - c1;
    c2 = 1.0 - c2;
    RGBToYUV(c0, c1, c2, y, u, v, (1.0 - 0.299),(1.0 - 0.587), (1.0 - 0.114));
}

QVector <double> CmykF32ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(5);
    channelValues.fill(1.0);
    YUVToRGB(*y, *u, *v, &channelValues[0],&channelValues[1],&channelValues[2], 0.33, 0.33, 0.33);
    channelValues[0] = qBound(0.0,1.0-channelValues[0],1.0);
    channelValues[1] = qBound(0.0,1.0-channelValues[1],1.0);
    channelValues[2] = qBound(0.0,1.0-channelValues[2],1.0);
    CMYToCMYK(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    return channelValues;
}
