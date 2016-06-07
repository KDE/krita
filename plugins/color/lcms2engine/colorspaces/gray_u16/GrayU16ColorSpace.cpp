/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "GrayU16ColorSpace.h"

#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "compositeops/KoCompositeOps.h"
#include <kis_dom_utils.h>

GrayAU16ColorSpace::GrayAU16ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<GrayAU16Traits>(colorSpaceId(), name,  TYPE_GRAYA_16, cmsSigGrayData, p)
{
    addChannel(new KoChannelInfo(i18n("Gray"), 0 * sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16));
    addChannel(new KoChannelInfo(i18n("Alpha"), 1 * sizeof(quint16), 1, KoChannelInfo::ALPHA, KoChannelInfo::UINT16));

    init();

    addStandardCompositeOps<GrayAU16Traits>(this);
}

KoColorSpace *GrayAU16ColorSpace::clone() const
{
    return new GrayAU16ColorSpace(name(), profile()->clone());
}

void GrayAU16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const GrayAU16Traits::channels_type *p = reinterpret_cast<const GrayAU16Traits::channels_type *>(pixel);
    QDomElement labElt = doc.createElement("Gray");
    labElt.setAttribute("g", KisDomUtils::toString(KoColorSpaceMaths< GrayAU16Traits::channels_type, qreal>::scaleToA(p[0])));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void GrayAU16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    GrayAU16Traits::channels_type *p = reinterpret_cast<GrayAU16Traits::channels_type *>(pixel);
    p[0] = KoColorSpaceMaths< qreal, GrayAU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("g")));
    p[1] = KoColorSpaceMathsTraits<quint16>::max;
}

void GrayAU16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *, qreal *, qreal *luma) const
{
    *luma = channelValues[0];
}

QVector <double> GrayAU16ColorSpace::fromHSY(qreal *, qreal *, qreal *luma) const
{
    QVector <double> channelValues(2);
    channelValues.fill(*luma);
    channelValues[1]=1.0;
    return channelValues;
}

void GrayAU16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *, qreal *) const
{
    *y = channelValues[0];
}

QVector <double> GrayAU16ColorSpace::fromYUV(qreal *y, qreal *, qreal *) const
{
    QVector <double> channelValues(2);
    channelValues.fill(*y);
    channelValues[1]=1.0;
    return channelValues;
}
