/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "RgbU16ColorSpace.h"

#include <QDomElement>

#include <klocale.h>

#include "compositeops/KoCompositeOps.h"
#include "compositeops/KoCompositeOpAdd.h"

#include "compositeops/RgbCompositeOps.h"

RgbU16ColorSpace::RgbU16ColorSpace(KoColorProfile *p) :
        LcmsColorSpace<KoRgbU16Traits>(colorSpaceId(), i18n("RGB (16-bit integer/channel)"),  TYPE_BGRA_16, icSigRgbData, p)
{
    addChannel(new KoChannelInfo(i18n("Red"),   2*sizeof(quint16), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT16, 2, QColor(255, 0, 0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1*sizeof(quint16), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT16, 2, QColor(0, 255, 0)));
    addChannel(new KoChannelInfo(i18n("Blue"),  0*sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16, 2, QColor(0, 0, 255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3*sizeof(quint16), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, 2));
    init();

    addStandardCompositeOps<KoRgbU16Traits>(this);

    addCompositeOp(new RgbCompositeOpDarken<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpLighten<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpHue<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpSaturation<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpValue<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpColor<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpIn<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpOut<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpDiff<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpBumpmap<KoRgbU16Traits>(this));
//     addCompositeOp(new RgbCompositeOpClear<KoRgbU16Traits>(this));
    addCompositeOp(new RgbCompositeOpDissolve<KoRgbU16Traits>(this));
}

bool RgbU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8)
        return true;
    else
        return false;
}

QString RgbU16ColorSpace::colorSpaceId()
{
    return QString("RGBA16");
}

KoColorSpace* RgbU16ColorSpace::clone() const
{
    return new RgbU16ColorSpace(profile()->clone());
}

void RgbU16ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoRgbU16Traits::Pixel* p = reinterpret_cast<const KoRgbU16Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("RGB");
    labElt.setAttribute("r", KoColorSpaceMaths< KoRgbU16Traits::channels_type, qreal>::scaleToA(p->red));
    labElt.setAttribute("g", KoColorSpaceMaths< KoRgbU16Traits::channels_type, qreal>::scaleToA(p->green));
    labElt.setAttribute("b", KoColorSpaceMaths< KoRgbU16Traits::channels_type, qreal>::scaleToA(p->blue));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void RgbU16ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoRgbU16Traits::Pixel* p = reinterpret_cast<KoRgbU16Traits::Pixel*>(pixel);
    p->red = KoColorSpaceMaths< qreal, KoRgbU16Traits::channels_type >::scaleToA(elt.attribute("r").toDouble());
    p->green = KoColorSpaceMaths< qreal, KoRgbU16Traits::channels_type >::scaleToA(elt.attribute("g").toDouble());
    p->blue = KoColorSpaceMaths< qreal, KoRgbU16Traits::channels_type >::scaleToA(elt.attribute("b").toDouble());
    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}

