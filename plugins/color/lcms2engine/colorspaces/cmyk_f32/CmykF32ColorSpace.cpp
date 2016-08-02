/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

CmykF32ColorSpace::CmykF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoCmykF32Traits>(colorSpaceId(), name,  TYPE_CMYKA_FLT, cmsSigCmykData, p)
{
    const IccColorProfile *icc_p = dynamic_cast<const IccColorProfile *>(p);
    Q_ASSERT(icc_p);
    QVector<KoChannelInfo::DoubleRange> uiRanges(icc_p->getFloatUIMinMax());
    Q_ASSERT(uiRanges.size() == 4);

    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(float), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::cyan, uiRanges[0]));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(float), 1, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::magenta, uiRanges[1]));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(float), 2, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::yellow, uiRanges[2]));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(float), 3, KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::black, uiRanges[3]));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(float), 4, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoCmykF32Traits>(this);
}

bool CmykF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *CmykF32ColorSpace::clone() const
{
    return new CmykF32ColorSpace(name(), profile()->clone());
}

void CmykF32ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoCmykF32Traits::Pixel *p = reinterpret_cast<const KoCmykF32Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("CMYK");
    labElt.setAttribute("c", KisDomUtils::toString(KoColorSpaceMaths<  KoCmykF32Traits::channels_type, qreal>::scaleToA(p->cyan)));
    labElt.setAttribute("m", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->magenta)));
    labElt.setAttribute("y", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->yellow)));
    labElt.setAttribute("k", KisDomUtils::toString(KoColorSpaceMaths< KoCmykF32Traits::channels_type, qreal>::scaleToA(p->black)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykF32ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoCmykF32Traits::Pixel *p = reinterpret_cast<KoCmykF32Traits::Pixel *>(pixel);
    p->cyan = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("c")));
    p->magenta = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("m")));
    p->yellow = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("y")));
    p->black = KoColorSpaceMaths< qreal, KoCmykF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("k")));
    p->alpha = 1.0;
}

void CmykF32ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    qreal c0 = channelValues[0];
    qreal c1 = channelValues[1];
    qreal c2 = channelValues[2];
    qreal c3 = channelValues[3];
    //we use HSI here because we can't linearise CMYK, and HSY doesn't work right with...
    CMYKToCMY(&c0, &c1, &c2, &c3);
    c0 = 1.0 - c0;
    c1 = 1.0 - c1;
    c2 = 1.0 - c2;
    RGBToHSI(c0, c1, c2, hue, sat, luma);
}

QVector <double> CmykF32ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(5);
    channelValues.fill(1.0);
    HSIToRGB(*hue, *sat, *luma, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[0] = qBound(0.0,1.0-channelValues[0],1.0);
    channelValues[1] = qBound(0.0,1.0-channelValues[1],1.0);
    channelValues[2] = qBound(0.0,1.0-channelValues[2],1.0);
    CMYToCMYK(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    return channelValues;
}

void CmykF32ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    qreal c0 = channelValues[0];
    qreal c1 = channelValues[1];
    qreal c2 = channelValues[2];
    qreal c3 = channelValues[3];
    CMYKToCMY(&c0, &c1, &c2, &c3);
    c0 = 1.0 - c0;
    c1 = 1.0 - c1;
    c2 = 1.0 - c2;
    RGBToYUV(c0, c1, c2, y, u, v, (1.0 - 0.299),(1.0 - 0.587), (1.0 - 0.114));
}

QVector <double> CmykF32ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(5);
    channelValues.fill(1.0);
    YUVToRGB(*y, *u, *v, &channelValues[0],&channelValues[1],&channelValues[2], 0.33, 0.33, 0.33);
    channelValues[0] = qBound(0.0,1.0-channelValues[0],1.0);
    channelValues[1] = qBound(0.0,1.0-channelValues[1],1.0);
    channelValues[2] = qBound(0.0,1.0-channelValues[2],1.0);
    CMYToCMYK(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    return channelValues;
}
