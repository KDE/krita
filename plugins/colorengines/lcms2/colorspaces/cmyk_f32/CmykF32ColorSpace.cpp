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

#include "CmykF32ColorSpace.h"

#include <QDomElement>
#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOps.h"

CmykF32ColorSpace::CmykF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoCmykF32Traits>(colorSpaceId(), name,  TYPE_CMYKA_FLT, cmsSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(float), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(float), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(float), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(float), 3, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(float), 4, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoCmykF32Traits>(this);
}

bool CmykF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16)
        return true;
    else
        return false;
}

KoColorSpace* CmykF32ColorSpace::clone() const
{
    return new CmykF32ColorSpace(name(), profile()->clone());
}

void CmykF32ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoCmykF32Traits::Pixel* p = reinterpret_cast<const KoCmykF32Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("CMYK");
    labElt.setAttribute("c", KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->cyan));
    labElt.setAttribute("m", KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->magenta));
    labElt.setAttribute("y", KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->yellow));
    labElt.setAttribute("k", KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->black));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykF32ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoCmykF32Traits::Pixel* p = reinterpret_cast<KoCmykF32Traits::Pixel*>(pixel);
    p->cyan = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(elt.attribute("c").toDouble());
    p->magenta = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(elt.attribute("m").toDouble());
    p->yellow = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(elt.attribute("y").toDouble());
    p->black = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(elt.attribute("k").toDouble());
    p->alpha = 1.0;
}


