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

#include "XyzU16ColorSpace.h"
#include <QDomElement>

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

XyzU16ColorSpace::XyzU16ColorSpace(KoColorProfile *p) :
        LcmsColorSpace<XyzU16Traits>("XYZA16", i18n("XYZ (16-bit integer/channel)"),  TYPE_XYZA_16, icSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("X"), XyzU16Traits::x_pos * sizeof(quint16), XyzU16Traits::x_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Y"), XyzU16Traits::y_pos * sizeof(quint16), XyzU16Traits::y_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Z"), XyzU16Traits::z_pos * sizeof(quint16), XyzU16Traits::z_pos, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), XyzU16Traits::alpha_pos * sizeof(quint16), XyzU16Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));
    init();

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<XyzU16Traits>(this);
}

bool XyzU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8)
        return true;
    else
        return false;
}

KoColorSpace* XyzU16ColorSpace::clone() const
{
    return new XyzU16ColorSpace(profile()->clone());
}

void XyzU16ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const XyzU16Traits::Pixel* p = reinterpret_cast<const XyzU16Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("XYZ");
    labElt.setAttribute("x", KoColorSpaceMaths< XyzU16Traits::channels_type, qreal>::scaleToA(p->X));
    labElt.setAttribute("y", KoColorSpaceMaths< XyzU16Traits::channels_type, qreal>::scaleToA(p->Y));
    labElt.setAttribute("z", KoColorSpaceMaths< XyzU16Traits::channels_type, qreal>::scaleToA(p->Z));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void XyzU16ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    XyzU16Traits::Pixel* p = reinterpret_cast<XyzU16Traits::Pixel*>(pixel);
    p->X = KoColorSpaceMaths< qreal, XyzU16Traits::channels_type >::scaleToA(elt.attribute("x").toDouble());
    p->Y = KoColorSpaceMaths< qreal, XyzU16Traits::channels_type >::scaleToA(elt.attribute("y").toDouble());
    p->Z = KoColorSpaceMaths< qreal, XyzU16Traits::channels_type >::scaleToA(elt.attribute("z").toDouble());
    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}

