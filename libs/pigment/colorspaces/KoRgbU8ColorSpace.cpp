/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "KoRgbU8ColorSpace.h"

#include <limits.h>
#include <stdlib.h>

#include <QImage>
#include <QBitArray>

#include <klocalizedstring.h>

#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"
#include "compositeops/KoCompositeOps.h"
#include "dithering/KisRgbDitherOpFactory.h"

#include "KoColorConversions.h"
#include <KoColorSpacePreserveLightnessUtils.h>

KoRgbU8ColorSpace::KoRgbU8ColorSpace() :

        KoSimpleColorSpace<KoBgrU8Traits>(colorSpaceId(),
                                          i18n("RGB (8-bit integer/channel, unmanaged)"),
                                          RGBAColorModelID,
                                          Integer8BitsColorDepthID)
{
    
    addChannel(new KoChannelInfo(i18n("Blue"),  0, 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 0, 255)));
    addChannel(new KoChannelInfo(i18n("Green"), 1, 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 255, 0)));
    addChannel(new KoChannelInfo(i18n("Red"),   2, 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255, 0, 0)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3, 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<KoBgrU8Traits>(this);
    addStandardDitherOps<KoBgrU8Traits>(this);
}

KoRgbU8ColorSpace::~KoRgbU8ColorSpace()
{
}


QString KoRgbU8ColorSpace::colorSpaceId()
{
    return QString("RGBA");
}


KoColorSpace* KoRgbU8ColorSpace::clone() const
{
    return new KoRgbU8ColorSpace();
}


void KoRgbU8ColorSpace::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
    QVector<float> channelValues;
    channelValues << c.blueF() << c.greenF() << c.redF() << c.alphaF();
    fromNormalisedChannelsValue(dst, channelValues);
}

void KoRgbU8ColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    QVector<float> channelValues(4);
    normalisedChannelsValue(src, channelValues);
    c->setRgbF(channelValues[2], channelValues[1], channelValues[0], channelValues[3]);
}

void KoRgbU8ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    
    RGBToHSY(channelValues[0],channelValues[1],channelValues[2], hue, sat, luma);
}

QVector <double> KoRgbU8ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    HSYToRGB(*hue, *sat, *luma, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void KoRgbU8ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    RGBToYUV(channelValues[0],channelValues[1],channelValues[2], y, u, v);
}

QVector <double> KoRgbU8ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    YUVToRGB(*y, *u, *v, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void KoRgbU8ColorSpace::fillGrayBrushWithColorAndLightnessOverlay(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const
{
    fillGrayBrushWithColorPreserveLightnessRGB<KoBgrU8Traits>(dst, brush, brushColor, 1.0, nPixels);
}

void KoRgbU8ColorSpace::fillGrayBrushWithColorAndLightnessWithStrength(quint8* dst, const QRgb* brush, quint8* brushColor, qreal strength, qint32 nPixels) const
{
    fillGrayBrushWithColorPreserveLightnessRGB<KoBgrU8Traits>(dst, brush, brushColor, strength, nPixels);
}

void KoRgbU8ColorSpace::modulateLightnessByGrayBrush(quint8 *dst, const QRgb *brush, quint8 *src, qreal strength, qint32 nPixels) const
{
   modulateLightnessByGrayBrushRGB<KoBgrU8Traits>(dst, brush, src, strength, nPixels);
}
