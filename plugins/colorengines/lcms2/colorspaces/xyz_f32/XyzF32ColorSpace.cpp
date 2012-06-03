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

#include "XyzF32ColorSpace.h"
#include <QDomElement>

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

XyzF32ColorSpace::XyzF32ColorSpace(const QString &name, KoColorProfile *p) :
    LcmsColorSpace<KoXyzF32Traits>(colorSpaceId(), name, TYPE_XYZA_FLT, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("X"),     KoXyzF32Traits::x_pos     * sizeof(float), KoXyzF32Traits::x_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Y"),     KoXyzF32Traits::y_pos     * sizeof(float), KoXyzF32Traits::y_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Z"),     KoXyzF32Traits::z_pos     * sizeof(float), KoXyzF32Traits::z_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoXyzF32Traits::alpha_pos * sizeof(float), KoXyzF32Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));
    init();

    addStandardCompositeOps<KoXyzF32Traits>(this);
}



bool XyzF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16)
        return true;
    else
        return false;
}

KoColorSpace* XyzF32ColorSpace::clone() const
{
    return new XyzF32ColorSpace(name(), profile()->clone());
}

void XyzF32ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoXyzF32Traits::Pixel* p = reinterpret_cast<const KoXyzF32Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("XYZ");
    labElt.setAttribute("x", KoColorSpaceMaths< KoXyzF32Traits::channels_type, qreal>::scaleToA(p->x));
    labElt.setAttribute("y", KoColorSpaceMaths< KoXyzF32Traits::channels_type, qreal>::scaleToA(p->y));
    labElt.setAttribute("z", KoColorSpaceMaths< KoXyzF32Traits::channels_type, qreal>::scaleToA(p->z));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void XyzF32ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoXyzF32Traits::Pixel* p = reinterpret_cast<KoXyzF32Traits::Pixel*>(pixel);
    p->x = KoColorSpaceMaths< qreal, KoXyzF32Traits::channels_type >::scaleToA(elt.attribute("x").toDouble());
    p->y = KoColorSpaceMaths< qreal, KoXyzF32Traits::channels_type >::scaleToA(elt.attribute("y").toDouble());
    p->z = KoColorSpaceMaths< qreal, KoXyzF32Traits::channels_type >::scaleToA(elt.attribute("z").toDouble());
    p->alpha = 1.0;
}

