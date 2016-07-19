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

#include "LabU8ColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include "../compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

LabU8ColorSpace::LabU8ColorSpace(const QString &name, KoColorProfile *p) :
    LcmsColorSpace<KoLabU8Traits>(colorSpaceId(), name, TYPE_LABA_8, cmsSigLabData, p)
{
    addChannel(new KoChannelInfo(i18n("Lightness"), 0 * sizeof(quint8), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(100, 100, 100)));
    addChannel(new KoChannelInfo(i18n("a*"),        1 * sizeof(quint8), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(150, 150, 150)));
    addChannel(new KoChannelInfo(i18n("b*"),        2 * sizeof(quint8), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(200, 200, 200)));
    addChannel(new KoChannelInfo(i18n("Alpha"),     3 * sizeof(quint8), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));
    init();
    addStandardCompositeOps<KoLabU8Traits>(this);
}

bool LabU8ColorSpace::willDegrade(ColorSpaceIndependence /*independence*/) const
{
    return false;
}

QString LabU8ColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    const KoLabU8Traits::channels_type *pix = reinterpret_cast<const  KoLabU8Traits::channels_type *>(pixel);
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
        return QString().setNum(100.0 * static_cast<float>(pix[3]) / UINT8_MAX);
    default:
        return QString("Error");
    }

}

KoColorSpace *LabU8ColorSpace::clone() const
{
    return new LabU8ColorSpace(name(), profile()->clone());
}

void LabU8ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoLabU8Traits::Pixel *p = reinterpret_cast<const KoLabU8Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("Lab");
    labElt.setAttribute("L", KisDomUtils::toString(KoColorSpaceMaths< KoLabU8Traits::channels_type, qreal>::scaleToA(p->L)));
    labElt.setAttribute("a", KisDomUtils::toString(KoColorSpaceMaths< KoLabU8Traits::channels_type, qreal>::scaleToA(p->a)));
    labElt.setAttribute("b", KisDomUtils::toString(KoColorSpaceMaths< KoLabU8Traits::channels_type, qreal>::scaleToA(p->b)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void LabU8ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoLabU8Traits::Pixel *p = reinterpret_cast<KoLabU8Traits::Pixel *>(pixel);
    p->L = KoColorSpaceMaths< qreal, KoLabU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("L")));
    p->a = KoColorSpaceMaths< qreal, KoLabU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("a")));
    p->b = KoColorSpaceMaths< qreal, KoLabU8Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("b")));
    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

void LabU8ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> LabU8ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void LabU8ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *u=channelValues[1];
    *v=channelValues[2];
}

QVector <double> LabU8ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*u;
    channelValues[2]=*v;
    channelValues[3]=1.0;
    return channelValues;
}
