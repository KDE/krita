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

#include "GrayF32ColorSpace.h"

#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "compositeops/KoCompositeOps.h"
#include <kis_dom_utils.h>

GrayF32ColorSpace::GrayF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoGrayF32Traits>(colorSpaceId(), name,  TYPE_GRAYA_FLT, cmsSigGrayData, p)
{
    const IccColorProfile *icc_p = dynamic_cast<const IccColorProfile *>(p);
    Q_ASSERT(icc_p);
    QVector<KoChannelInfo::DoubleRange> uiRanges(icc_p->getFloatUIMinMax());
    Q_ASSERT(uiRanges.size() == 1);

    addChannel(new KoChannelInfo(i18n("Gray"), 0 * sizeof(float), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::gray, uiRanges[0]));
    addChannel(new KoChannelInfo(i18n("Alpha"), 1 * sizeof(float), 1, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoGrayF32Traits>(this);
}

KoColorSpace *GrayF32ColorSpace::clone() const
{
    return new GrayF32ColorSpace(name(), profile()->clone());
}

void GrayF32ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoGrayF32Traits::channels_type *p = reinterpret_cast<const KoGrayF32Traits::channels_type *>(pixel);
    QDomElement labElt = doc.createElement("Gray");
    labElt.setAttribute("g", KisDomUtils::toString(KoColorSpaceMaths< KoGrayF32Traits::channels_type, qreal>::scaleToA(p[0])));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void GrayF32ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoGrayF32Traits::channels_type *p = reinterpret_cast<KoGrayF32Traits::channels_type *>(pixel);
    p[0] = KoColorSpaceMaths< qreal, KoGrayF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("g")));
    p[1] = 1.0;
}

void GrayF32ColorSpace::toHSY(const QVector<double> &channelValues, qreal *, qreal *, qreal *luma) const
{
    *luma = channelValues[0];
}

QVector <double> GrayF32ColorSpace::fromHSY(qreal *, qreal *, qreal *luma) const
{
    QVector <double> channelValues(2);
    channelValues.fill(*luma);
    channelValues[1]=1.0;
    return channelValues;
}

void GrayF32ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *, qreal *) const
{
    *y = channelValues[0];
}

QVector <double> GrayF32ColorSpace::fromYUV(qreal *y, qreal *, qreal *) const
{
    QVector <double> channelValues(2);
    channelValues.fill(*y);
    channelValues[1]=1.0;
    return channelValues;
}
