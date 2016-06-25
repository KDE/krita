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

    addChannel(new KoChannelInfo(i18n("Lightness"), 0 * sizeof(float), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), QColor(100, 100, 100), uiRanges[0]));
    addChannel(new KoChannelInfo(i18n("a*"),        1 * sizeof(float), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), QColor(150, 150, 150), uiRanges[1]));
    addChannel(new KoChannelInfo(i18n("b*"),        2 * sizeof(float), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), QColor(200, 200, 200), uiRanges[2]));
    addChannel(new KoChannelInfo(i18n("Alpha"),     3 * sizeof(float), 3, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoLabF32Traits>(this);
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
