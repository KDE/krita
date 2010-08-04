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

#include <kdebug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "compositeops/KoCompositeOps.h"

KoGrayAU16ColorSpace ::KoGrayAU16ColorSpace(KoColorProfile *p) :
        LcmsColorSpace<GrayAU16Traits>("GRAYA16", i18n("Grayscale (16-bit integer/channel)"),  TYPE_GRAYA_16, icSigGrayData, p)
{
    addChannel(new KoChannelInfo(i18n("Gray"), 0 * sizeof(quint16), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT16));
    addChannel(new KoChannelInfo(i18n("Alpha"), 1 * sizeof(quint16), 1, KoChannelInfo::ALPHA, KoChannelInfo::UINT16));

    init();

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<GrayAU16Traits>(this);
}

KoColorSpace* KoGrayAU16ColorSpace::clone() const
{
    return new KoGrayAU16ColorSpace(profile()->clone());
}

void KoGrayAU16ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const GrayAU16Traits::channels_type* p = reinterpret_cast<const GrayAU16Traits::channels_type*>(pixel);
    QDomElement labElt = doc.createElement("Gray");
    labElt.setAttribute("g", KoColorSpaceMaths< GrayAU16Traits::channels_type, qreal>::scaleToA(p[0]));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void KoGrayAU16ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    GrayAU16Traits::channels_type* p = reinterpret_cast<GrayAU16Traits::channels_type*>(pixel);
    p[0] = KoColorSpaceMaths< qreal, GrayAU16Traits::channels_type >::scaleToA(elt.attribute("g").toDouble());
    p[1] = KoColorSpaceMathsTraits<quint16>::max;
}

