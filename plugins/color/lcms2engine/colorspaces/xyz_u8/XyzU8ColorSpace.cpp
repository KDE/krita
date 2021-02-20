/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger (cberger@cberger.net)
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "XyzU8ColorSpace.h"
#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include "dithering/KisXyzDitherOpFactory.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

XyzU8ColorSpace::XyzU8ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoXyzU8Traits>(colorSpaceId(), name, TYPE_XYZA_8, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("X"), KoXyzU8Traits::x_pos * sizeof(quint8), KoXyzU8Traits::x_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Y"), KoXyzU8Traits::y_pos * sizeof(quint8), KoXyzU8Traits::y_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Z"), KoXyzU8Traits::z_pos * sizeof(quint8), KoXyzU8Traits::z_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoXyzU8Traits::alpha_pos * sizeof(quint8), KoXyzU8Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));

    init();

    addStandardCompositeOps<KoXyzU8Traits>(this);
    addStandardDitherOps<KoXyzU8Traits>(this);
}

bool XyzU8ColorSpace::willDegrade(ColorSpaceIndependence /*independence*/) const
{
    return false;
}

KoColorSpace *XyzU8ColorSpace::clone() const
{
    return new XyzU8ColorSpace(name(), profile()->clone());
}

void XyzU8ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoXyzU8Traits::Pixel *p = reinterpret_cast<const KoXyzU8Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("XYZ");
    labElt.setAttribute("x", KisDomUtils::toString(KoColorSpaceMaths< KoXyzU8Traits::channels_type, qreal>::scaleToA(p->x)));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths< KoXyzU8Traits::channels_type, qreal>::scaleToA(p->y)));
    labElt.setAttribute("z", KisDomUtils::toString(KoColorSpaceMaths< KoXyzU8Traits::channels_type, qreal>::scaleToA(p->z)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void XyzU8ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoXyzU8Traits::Pixel *p = reinterpret_cast<KoXyzU8Traits::Pixel *>(pixel);
    p->x = KoColorSpaceMaths< qreal, KoXyzU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("x")));
    p->y = KoColorSpaceMaths< qreal, KoXyzU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y")));
    p->z = KoColorSpaceMaths< qreal, KoXyzU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("z")));
    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

void XyzU8ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    qreal xyx, xyy, xyY = 0.0;
    XYZToxyY(channelValues[0],channelValues[1],channelValues[2], &xyx, &xyy, &xyY);
    LabToLCH(xyY,xyx,xyY, hue, sat, luma);
}

QVector <double> XyzU8ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    qreal xyx, xyy, xyY = 0.0;
    LCHToLab(*luma, *sat, *hue, &xyY,&xyx,&xyy);
    xyYToXYZ(xyx, xyy, xyY, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void XyzU8ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    XYZToxyY(channelValues[0],channelValues[1],channelValues[2], u, v, y);
}

QVector <double> XyzU8ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    xyYToXYZ(*u, *v, *y, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}
