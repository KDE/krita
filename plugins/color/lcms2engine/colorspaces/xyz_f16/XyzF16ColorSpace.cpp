/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger (cberger@cberger.net)
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "XyzF16ColorSpace.h"
#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include "dithering/KisXyzDitherOpFactory.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

XyzF16ColorSpace::XyzF16ColorSpace(const QString &name, KoColorProfile *p) :
    LcmsColorSpace<KoXyzF16Traits>(colorSpaceId(), name, TYPE_XYZA_HALF_FLT, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("X"),     KoXyzF16Traits::x_pos     * sizeof(half), KoXyzF16Traits::x_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Y"),     KoXyzF16Traits::y_pos     * sizeof(half), KoXyzF16Traits::y_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Z"),     KoXyzF16Traits::z_pos     * sizeof(half), KoXyzF16Traits::z_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoXyzF16Traits::alpha_pos * sizeof(half), KoXyzF16Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT16, 2));

    init();

    addStandardCompositeOps<KoXyzF16Traits>(this);
    addStandardDitherOps<KoXyzF16Traits>(this);
}

bool XyzF16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *XyzF16ColorSpace::clone() const
{
    return new XyzF16ColorSpace(name(), profile()->clone());
}

void XyzF16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoXyzF16Traits::Pixel *p = reinterpret_cast<const KoXyzF16Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("XYZ");
    labElt.setAttribute("x", KisDomUtils::toString(KoColorSpaceMaths< KoXyzF16Traits::channels_type, qreal>::scaleToA(p->x)));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths< KoXyzF16Traits::channels_type, qreal>::scaleToA(p->y)));
    labElt.setAttribute("z", KisDomUtils::toString(KoColorSpaceMaths< KoXyzF16Traits::channels_type, qreal>::scaleToA(p->z)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void XyzF16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoXyzF16Traits::Pixel *p = reinterpret_cast<KoXyzF16Traits::Pixel *>(pixel);
    p->x = KoColorSpaceMaths< qreal, KoXyzF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("x")));
    p->y = KoColorSpaceMaths< qreal, KoXyzF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y")));
    p->z = KoColorSpaceMaths< qreal, KoXyzF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("z")));
    p->alpha = 1.0;
}

void XyzF16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    qreal xyx, xyy, xyY = 0.0;
    XYZToxyY(channelValues[0],channelValues[1],channelValues[2], &xyx, &xyy, &xyY);
    LabToLCH(xyY,xyx,xyY, hue, sat, luma);
}

QVector <double> XyzF16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    qreal xyx, xyy, xyY = 0.0;
    LCHToLab(*luma, *sat, *hue, &xyY,&xyx,&xyy);
    xyYToXYZ(xyx, xyy, xyY, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void XyzF16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    XYZToxyY(channelValues[0],channelValues[1],channelValues[2], u, v, y);
}

QVector <double> XyzF16ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    xyYToXYZ(*u, *v, *y, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}
