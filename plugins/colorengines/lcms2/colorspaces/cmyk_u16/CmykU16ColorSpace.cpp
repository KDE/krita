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

#include "CmykU16ColorSpace.h"

#include <QDomElement>
#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

CmykU16ColorSpace::CmykU16ColorSpace(KoColorProfile *p) :
        LcmsColorSpace<CmykU16Traits>("CMYKA16", i18n("CMYK (16-bit integer/channel)"),  TYPE_CMYK5_16, cmsSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint16), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint16), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint16), 3, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint16), 4, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));
    init();

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<CmykU16Traits>(this);
}

bool CmykU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8)
        return true;
    else
        return false;
}

KoColorSpace* CmykU16ColorSpace::clone() const
{
    return new CmykU16ColorSpace(profile()->clone());
}

void CmykU16ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const CmykU16Traits::Pixel* p = reinterpret_cast<const CmykU16Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("CMYK");
    labElt.setAttribute("c", KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->cyan));
    labElt.setAttribute("m", KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->magenta));
    labElt.setAttribute("y", KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->yellow));
    labElt.setAttribute("k", KoColorSpaceMaths< CmykU16Traits::channels_type, qreal>::scaleToA(p->black));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykU16ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    CmykU16Traits::Pixel* p = reinterpret_cast<CmykU16Traits::Pixel*>(pixel);
    p->cyan = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(elt.attribute("c").toDouble());
    p->magenta = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(elt.attribute("m").toDouble());
    p->yellow = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(elt.attribute("y").toDouble());
    p->black = KoColorSpaceMaths< qreal, CmykU16Traits::channels_type >::scaleToA(elt.attribute("k").toDouble());
    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}

