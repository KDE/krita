/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger (cberger@cberger.net)
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "XyzU16ColorSpace.h"
#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include "dithering/KisXyzDitherOpFactory.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

XyzU16ColorSpace::XyzU16ColorSpace(const QString &name, KoColorProfile *p) :
    LcmsColorSpace<KoXyzU16Traits>(colorSpaceId(), name, TYPE_XYZA_16, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("X"), KoXyzU16Traits::x_pos * sizeof(quint16), KoXyzU16Traits::x_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Y"), KoXyzU16Traits::y_pos * sizeof(quint16), KoXyzU16Traits::y_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Z"), KoXyzU16Traits::z_pos * sizeof(quint16), KoXyzU16Traits::z_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoXyzU16Traits::alpha_pos * sizeof(quint16), KoXyzU16Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));
    init();

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<KoXyzU16Traits>(this);
    addStandardDitherOps<KoXyzU16Traits>(this);
}

bool XyzU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *XyzU16ColorSpace::clone() const
{
    return new XyzU16ColorSpace(name(), profile()->clone());
}

void XyzU16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoXyzU16Traits::Pixel *p = reinterpret_cast<const KoXyzU16Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("XYZ");
    labElt.setAttribute("x", KisDomUtils::toString(KoColorSpaceMaths< KoXyzU16Traits::channels_type, qreal>::scaleToA(p->x)));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths< KoXyzU16Traits::channels_type, qreal>::scaleToA(p->y)));
    labElt.setAttribute("z", KisDomUtils::toString(KoColorSpaceMaths< KoXyzU16Traits::channels_type, qreal>::scaleToA(p->z)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void XyzU16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoXyzU16Traits::Pixel *p = reinterpret_cast<KoXyzU16Traits::Pixel *>(pixel);
    p->x = KoColorSpaceMaths< qreal, KoXyzU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("x")));
    p->y = KoColorSpaceMaths< qreal, KoXyzU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y")));
    p->z = KoColorSpaceMaths< qreal, KoXyzU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("z")));
    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}

void XyzU16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    qreal xyx, xyy, xyY = 0.0;
    XYZToxyY(channelValues[0],channelValues[1],channelValues[2], &xyx, &xyy, &xyY);
    LabToLCH(xyY,xyx,xyY, hue, sat, luma);
}

QVector <double> XyzU16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    qreal xyx, xyy, xyY = 0.0;
    LCHToLab(*luma, *sat, *hue, &xyY,&xyx,&xyy);
    xyYToXYZ(xyx, xyy, xyY, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void XyzU16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    XYZToxyY(channelValues[0],channelValues[1],channelValues[2], u, v, y);
}

QVector <double> XyzU16ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    xyYToXYZ(*u, *v, *y, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}
