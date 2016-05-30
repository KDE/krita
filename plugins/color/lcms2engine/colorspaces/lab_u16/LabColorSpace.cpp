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

#include "LabColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include "../compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

LabU16ColorSpace::LabU16ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoLabU16Traits>(colorSpaceId(), name, TYPE_LABA_16, cmsSigLabData, p)
{
    addChannel(new KoChannelInfo(i18n("Lightness"), 0 * sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(100, 100, 100)));
    addChannel(new KoChannelInfo(i18n("a*"),        1 * sizeof(quint16), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(150, 150, 150)));
    addChannel(new KoChannelInfo(i18n("b*"),        2 * sizeof(quint16), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(200, 200, 200)));
    addChannel(new KoChannelInfo(i18n("Alpha"),     3 * sizeof(quint16), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    init();

    addStandardCompositeOps<KoLabU16Traits>(this);
}

bool LabU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) {
        return true;
    } else {
        return false;
    }
}

QString LabU16ColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    const KoLabU16Traits::channels_type *pix = reinterpret_cast<const  KoLabU16Traits::channels_type *>(pixel);
    Q_ASSERT(channelIndex < channelCount());

    // These convert from lcms encoded format to standard ranges.

    switch (channelIndex) {
    case 0:
        return QString().setNum(100.0 * static_cast<float>(pix[0]) / MAX_CHANNEL_L);
    case 1:
        return QString().setNum(100.0 * ((static_cast<float>(pix[1]) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
    case 2:
        return QString().setNum(100.0 * ((static_cast<float>(pix[2]) - CHANNEL_AB_ZERO_OFFSET) / MAX_CHANNEL_AB));
    case 3:
        return QString().setNum(100.0 * static_cast<float>(pix[3]) / UINT16_MAX);
    default:
        return QString("Error");
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
    labElt.setAttribute("L", KisDomUtils::toString(KoColorSpaceMaths< KoLabU16Traits::channels_type, qreal>::scaleToA(p->L)));
    labElt.setAttribute("a", KisDomUtils::toString(KoColorSpaceMaths< KoLabU16Traits::channels_type, qreal>::scaleToA(p->a)));
    labElt.setAttribute("b", KisDomUtils::toString(KoColorSpaceMaths< KoLabU16Traits::channels_type, qreal>::scaleToA(p->b)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void LabU16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoLabU16Traits::Pixel *p = reinterpret_cast<KoLabU16Traits::Pixel *>(pixel);
    p->L = KoColorSpaceMaths< qreal, KoLabU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("L")));
    p->a = KoColorSpaceMaths< qreal, KoLabU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("a")));
    p->b = KoColorSpaceMaths< qreal, KoLabU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("b")));
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
