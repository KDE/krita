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

#include "YCbCrF32ColorSpace.h"
#include <QDomElement>

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

YCbCrF32ColorSpace::YCbCrF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoYCbCrF32Traits>(colorSpaceId(), name, TYPE_YCbCrA_FLT, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("Y"),     KoYCbCrF32Traits::Y_pos     * sizeof(float), KoYCbCrF32Traits::Y_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Cb"),    KoYCbCrF32Traits::Cb_pos    * sizeof(float), KoYCbCrF32Traits::Cb_pos,    KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Cr"),    KoYCbCrF32Traits::Cr_pos    * sizeof(float), KoYCbCrF32Traits::Cr_pos,    KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoYCbCrF32Traits::alpha_pos * sizeof(float), KoYCbCrF32Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoYCbCrF32Traits>(this);
}

bool YCbCrF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16)
        return true;
    else
        return false;

}

KoColorSpace* YCbCrF32ColorSpace::clone() const
{
    return new YCbCrF32ColorSpace(name(), profile()->clone());
}

void YCbCrF32ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoYCbCrF32Traits::Pixel* p = reinterpret_cast<const KoYCbCrF32Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("YCbCr");
    labElt.setAttribute("Y",  KoColorSpaceMaths< KoYCbCrF32Traits::channels_type, qreal>::scaleToA(p->Y));
    labElt.setAttribute("Cb", KoColorSpaceMaths< KoYCbCrF32Traits::channels_type, qreal>::scaleToA(p->Cb));
    labElt.setAttribute("Cr", KoColorSpaceMaths< KoYCbCrF32Traits::channels_type, qreal>::scaleToA(p->Cr));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void YCbCrF32ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoYCbCrF32Traits::Pixel* p = reinterpret_cast<KoYCbCrF32Traits::Pixel*>(pixel);
    p->Y = KoColorSpaceMaths< qreal, KoYCbCrF32Traits::channels_type >::scaleToA(elt.attribute("Y").toDouble());
    p->Cb = KoColorSpaceMaths< qreal, KoYCbCrF32Traits::channels_type >::scaleToA(elt.attribute("Cb").toDouble());
    p->Cr = KoColorSpaceMaths< qreal, KoYCbCrF32Traits::channels_type >::scaleToA(elt.attribute("Cr").toDouble());
    p->alpha = 1.0;
}

