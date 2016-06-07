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

#include "GrayF16ColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "compositeops/KoCompositeOps.h"
#include <kis_dom_utils.h>

GrayF16ColorSpace::GrayF16ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoGrayF16Traits>(colorSpaceId(), name,  TYPE_GRAYA_HALF_FLT, cmsSigGrayData, p)
{
    const IccColorProfile *icc_p = dynamic_cast<const IccColorProfile *>(p);
    Q_ASSERT(icc_p);
    Q_UNUSED(icc_p);
    addChannel(new KoChannelInfo(i18n("Gray"), 0 * sizeof(half), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, 2, Qt::gray));
    addChannel(new KoChannelInfo(i18n("Alpha"), 1 * sizeof(half), 1, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT16, 2));

    init();

    addStandardCompositeOps<KoGrayF16Traits>(this);
}

KoColorSpace *GrayF16ColorSpace::clone() const
{
    return new GrayF16ColorSpace(name(), profile()->clone());
}

void GrayF16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoGrayF16Traits::channels_type *p = reinterpret_cast<const KoGrayF16Traits::channels_type *>(pixel);
    QDomElement labElt = doc.createElement("Gray");
    labElt.setAttribute("g", KisDomUtils::toString(KoColorSpaceMaths< KoGrayF16Traits::channels_type, qreal>::scaleToA(p[0])));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void GrayF16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoGrayF16Traits::channels_type *p = reinterpret_cast<KoGrayF16Traits::channels_type *>(pixel);
    p[0] = KoColorSpaceMaths< qreal, KoGrayF16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("g")));
    p[1] = 1.0;
}

void GrayF16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *, qreal *, qreal *luma) const
{
    *luma = channelValues[0];
}

QVector <double> GrayF16ColorSpace::fromHSY(qreal *, qreal *, qreal *luma) const
{
    QVector <double> channelValues(2);
    channelValues.fill(*luma);
    channelValues[1]=1.0;
    return channelValues;
}

void GrayF16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *, qreal *) const
{
    *y = channelValues[0];
}

QVector <double> GrayF16ColorSpace::fromYUV(qreal *y, qreal *, qreal *) const
{
    QVector <double> channelValues(2);
    channelValues.fill(*y);
    channelValues[1]=1.0;
    return channelValues;
}
