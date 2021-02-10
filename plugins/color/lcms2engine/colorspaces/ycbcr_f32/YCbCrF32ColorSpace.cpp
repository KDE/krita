/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger (cberger@cberger.net)
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "YCbCrF32ColorSpace.h"
#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include "dithering/KisYCbCrDitherOpFactory.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

YCbCrF32ColorSpace::YCbCrF32ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoYCbCrF32Traits>(colorSpaceId(), name, TYPE_YCbCrA_FLT, cmsSigXYZData, p)
{
    const IccColorProfile *icc_p = dynamic_cast<const IccColorProfile *>(p);
    Q_ASSERT(icc_p);
    QVector<KoChannelInfo::DoubleRange> uiRanges(icc_p->getFloatUIMinMax());
    Q_ASSERT(uiRanges.size() == 3);

    addChannel(new KoChannelInfo(i18n("Y"),     KoYCbCrF32Traits::Y_pos     * sizeof(float), KoYCbCrF32Traits::Y_pos,     KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::cyan, uiRanges[0]));
    addChannel(new KoChannelInfo(i18n("Cb"),    KoYCbCrF32Traits::Cb_pos    * sizeof(float), KoYCbCrF32Traits::Cb_pos,    KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::magenta, uiRanges[1]));
    addChannel(new KoChannelInfo(i18n("Cr"),    KoYCbCrF32Traits::Cr_pos    * sizeof(float), KoYCbCrF32Traits::Cr_pos,    KoChannelInfo::COLOR, KoChannelInfo::FLOAT32, sizeof(float), Qt::yellow, uiRanges[2]));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoYCbCrF32Traits::alpha_pos * sizeof(float), KoYCbCrF32Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32, sizeof(float)));

    init();

    addStandardCompositeOps<KoYCbCrF32Traits>(this);
    addStandardDitherOps<KoYCbCrF32Traits>(this);
}

bool YCbCrF32ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA16) {
        return true;
    } else {
        return false;
    }

}

KoColorSpace *YCbCrF32ColorSpace::clone() const
{
    return new YCbCrF32ColorSpace(name(), profile()->clone());
}

void YCbCrF32ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoYCbCrF32Traits::Pixel *p = reinterpret_cast<const KoYCbCrF32Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("YCbCr");
    labElt.setAttribute("Y",  KisDomUtils::toString(KoColorSpaceMaths< KoYCbCrF32Traits::channels_type, qreal>::scaleToA(p->Y)));
    labElt.setAttribute("Cb", KisDomUtils::toString(KoColorSpaceMaths< KoYCbCrF32Traits::channels_type, qreal>::scaleToA(p->Cb)));
    labElt.setAttribute("Cr", KisDomUtils::toString(KoColorSpaceMaths< KoYCbCrF32Traits::channels_type, qreal>::scaleToA(p->Cr)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void YCbCrF32ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoYCbCrF32Traits::Pixel *p = reinterpret_cast<KoYCbCrF32Traits::Pixel *>(pixel);
    p->Y = KoColorSpaceMaths< qreal, KoYCbCrF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("Y")));
    p->Cb = KoColorSpaceMaths< qreal, KoYCbCrF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("Cb")));
    p->Cr = KoColorSpaceMaths< qreal, KoYCbCrF32Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("Cr")));
    p->alpha = 1.0;
}

void YCbCrF32ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> YCbCrF32ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void YCbCrF32ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *u=channelValues[1];
    *v=channelValues[2];
}

QVector <double> YCbCrF32ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*u;
    channelValues[2]=*v;
    channelValues[3]=1.0;
    return channelValues;
}
