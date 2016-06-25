/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "CmykU16ColorSpace.h"

#include <QDomElement>
#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

CmykU16ColorSpace::CmykU16ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<CmykU16Traits>(colorSpaceId(), name,  TYPE_CMYKA_16, cmsSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint16), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint16), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint16), 3, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint16), 4, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    init();

    addStandardCompositeOps<CmykU16Traits>(this);
}

bool CmykU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *CmykU16ColorSpace::clone() const
{
    return new CmykU16ColorSpace(name(), profile()->clone());
}

void CmykU16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const CmykU16Traits::Pixel *p = reinterpret_cast<const CmykU16Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("CMYK");
    labElt.setAttribute("c", KisDomUtils::toString(KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->cyan)));
    labElt.setAttribute("m", KisDomUtils::toString(KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->magenta)));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->yellow)));
    labElt.setAttribute("k", KisDomUtils::toString(KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->black)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykU16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    CmykU16Traits::Pixel *p = reinterpret_cast<CmykU16Traits::Pixel *>(pixel);
    p->cyan = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("c")));
    p->magenta = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("m")));
    p->yellow = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y")));
    p->black = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("k")));
    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}

void CmykU16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
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

QVector <double> CmykU16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
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

void CmykU16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
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

QVector <double> CmykU16ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
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
