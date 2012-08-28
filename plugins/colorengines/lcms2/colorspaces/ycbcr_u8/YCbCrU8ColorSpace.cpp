/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "YCbCrU8ColorSpace.h"
#include <QDomElement>

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

YCbCrU8ColorSpace::YCbCrU8ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoYCbCrU8Traits>(colorSpaceId(), name, TYPE_YCbCrA_8, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("Y"),     KoYCbCrU8Traits::Y_pos     * sizeof(quint8), KoYCbCrU8Traits::Y_pos,     KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Cb"),    KoYCbCrU8Traits::Cb_pos    * sizeof(quint8), KoYCbCrU8Traits::Cb_pos,    KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Cr"),    KoYCbCrU8Traits::Cr_pos    * sizeof(quint8), KoYCbCrU8Traits::Cr_pos,    KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoYCbCrU8Traits::alpha_pos * sizeof(quint8), KoYCbCrU8Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));

    init();

    addStandardCompositeOps<KoYCbCrU8Traits>(this);
}

bool YCbCrU8ColorSpace::willDegrade(ColorSpaceIndependence /*independence*/) const
{
    return false;
}

KoColorSpace* YCbCrU8ColorSpace::clone() const
{
    return new YCbCrU8ColorSpace(name(), profile()->clone());
}

void YCbCrU8ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoYCbCrU8Traits::Pixel* p = reinterpret_cast<const KoYCbCrU8Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("YCbCr");
    labElt.setAttribute("Y",  KoColorSpaceMaths< KoYCbCrU8Traits::channels_type, qreal>::scaleToA(p->Y));
    labElt.setAttribute("Cb", KoColorSpaceMaths< KoYCbCrU8Traits::channels_type, qreal>::scaleToA(p->Cb));
    labElt.setAttribute("Cr", KoColorSpaceMaths< KoYCbCrU8Traits::channels_type, qreal>::scaleToA(p->Cr));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void YCbCrU8ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoYCbCrU8Traits::Pixel* p = reinterpret_cast<KoYCbCrU8Traits::Pixel*>(pixel);
    p->Y = KoColorSpaceMaths< qreal, KoYCbCrU8Traits::channels_type >::scaleToA(elt.attribute("Y").toDouble());
    p->Cb = KoColorSpaceMaths< qreal, KoYCbCrU8Traits::channels_type >::scaleToA(elt.attribute("Cb").toDouble());
    p->Cr = KoColorSpaceMaths< qreal, KoYCbCrU8Traits::channels_type >::scaleToA(elt.attribute("Cr").toDouble());
    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

