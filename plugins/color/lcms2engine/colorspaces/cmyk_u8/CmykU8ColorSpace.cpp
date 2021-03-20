/*
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "CmykU8ColorSpace.h"

#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include "dithering/KisCmykDitherOpFactory.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

CmykU8ColorSpace::CmykU8ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoCmykU8Traits>(colorSpaceId(), name, TYPE_CMYKA_8, cmsSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint8), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint8), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint8), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint8), 3, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint8), 4, KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));

    init();

    addStandardCompositeOps<KoCmykU8Traits>(this);
    addStandardDitherOps<KoCmykU8Traits>(this);
}

bool CmykU8ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *CmykU8ColorSpace::clone() const
{
    return new CmykU8ColorSpace(name(), profile()->clone());
}

void CmykU8ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoCmykU8Traits::Pixel *p = reinterpret_cast<const KoCmykU8Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("CMYK");
    labElt.setAttribute("c", KisDomUtils::toString(KoColorSpaceMaths<KoCmykU8Traits::channels_type, qreal>::scaleToA(p->cyan)));
    labElt.setAttribute("m", KisDomUtils::toString(KoColorSpaceMaths<KoCmykU8Traits::channels_type, qreal>::scaleToA(p->magenta)));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths<KoCmykU8Traits::channels_type, qreal>::scaleToA(p->yellow)));
    labElt.setAttribute("k", KisDomUtils::toString(KoColorSpaceMaths<KoCmykU8Traits::channels_type, qreal>::scaleToA(p->black)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykU8ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoCmykU8Traits::Pixel *p = reinterpret_cast<KoCmykU8Traits::Pixel *>(pixel);
    p->cyan = KoColorSpaceMaths< qreal, KoCmykU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("c")));
    p->magenta = KoColorSpaceMaths< qreal, KoCmykU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("m")));
    p->yellow = KoColorSpaceMaths< qreal, KoCmykU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y")));
    p->black = KoColorSpaceMaths< qreal, KoCmykU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("k")));
    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

void CmykU8ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
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

QVector <double> CmykU8ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
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

void CmykU8ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    qreal c0 = channelValues[0];
    qreal c1 = channelValues[1];
    qreal c2 = channelValues[2];
    qreal c3 = channelValues[3];
    CMYKToCMY(&c0, &c1, &c2, &c3);
    c0 = 1.0 - c0;
    c1 = 1.0 - c1;
    c2 = 1.0 - c2;
    RGBToYUV(c0, c1, c2, y, u, v, 0.33, 0.33, 0.33);
}

QVector <double> CmykU8ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
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
