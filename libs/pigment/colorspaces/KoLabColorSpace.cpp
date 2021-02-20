/*
 *  SPDX-FileCopyrightText: 2004-2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "KoLabColorSpace.h"

#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include <QImage>
#include <QBitArray>

#include <klocalizedstring.h>

#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"
#include "KoColorConversions.h"

#include "../compositeops/KoCompositeOps.h"
#include "dithering/KisLabDitherOpFactory.h"

KoLabColorSpace::KoLabColorSpace() :
        KoSimpleColorSpace<KoLabU16Traits>(colorSpaceId(),
                                           i18n("L*a*b* (16-bit integer/channel, unmanaged)"),
                                           LABAColorModelID,
                                           Integer16BitsColorDepthID)
{
    addChannel(new KoChannelInfo(i18nc("Lightness value in Lab color model", "Lightness"), CHANNEL_L     * sizeof(quint16), CHANNEL_L, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(100, 100, 100)));
    addChannel(new KoChannelInfo(i18n("a*"),        CHANNEL_A     * sizeof(quint16), CHANNEL_A, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(150, 150, 150)));
    addChannel(new KoChannelInfo(i18n("b*"),        CHANNEL_B     * sizeof(quint16), CHANNEL_B, KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(200, 200, 200)));
    addChannel(new KoChannelInfo(i18n("Alpha"),     CHANNEL_ALPHA * sizeof(quint16), CHANNEL_ALPHA, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<KoLabU16Traits>(this);
    addStandardDitherOps<KoLabU16Traits>(this);
}

KoLabColorSpace::~KoLabColorSpace()
{
}


QString KoLabColorSpace::colorSpaceId()
{
    return QString("LABA");
}


KoColorSpace* KoLabColorSpace::clone() const
{
    return new KoLabColorSpace();
}

void KoLabColorSpace::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
    // Convert between RGB and CIE-Lab color spaces
    // Uses ITU-R recommendation BT.709 with D65 as reference white.
    // algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>

    int R, G, B, A;
    c.getRgb(&R, &G, &B, &A);

    double X, Y, Z, fX, fY, fZ;

    X = 0.412453 * R + 0.357580 * G + 0.180423 * B;
    Y = 0.212671 * R + 0.715160 * G + 0.072169 * B;
    Z = 0.019334 * R + 0.119193 * G + 0.950227 * B;

    X /= (255 * 0.950456);
    Y /=  255;
    Z /= (255 * 1.088754);

    quint8 L, a, b;

    if (Y > 0.008856) {
        fY = pow(Y, 1.0 / 3.0);
        L = static_cast<int>(116.0 * fY - 16.0 + 0.5);
    } else {
        fY = 7.787 * Y + 16.0 / 116.0;
        L = static_cast<int>(903.3 * Y + 0.5);
    }

    if (X > 0.008856)
        fX = pow(X, 1.0 / 3.0);
    else
        fX = 7.787 * X + 16.0 / 116.0;

    if (Z > 0.008856)
        fZ = pow(Z, 1.0 / 3.0);
    else
        fZ = 7.787 * Z + 16.0 / 116.0;

    a = static_cast<int>(500.0 * (fX - fY) + 0.5);
    b = static_cast<int>(200.0 * (fY - fZ) + 0.5);

    dst[CHANNEL_L] = UINT8_TO_UINT16(L);
    dst[CHANNEL_A] = UINT8_TO_UINT16(a);
    dst[CHANNEL_B] = UINT8_TO_UINT16(b);
    dst[CHANNEL_ALPHA] = UINT8_TO_UINT16(A);
}

void KoLabColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    // Convert between RGB and CIE-Lab color spaces
    // Uses ITU-R recommendation BT.709 with D65 as reference white.
    // algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>
    quint8 L, a, b, A;
    L = UINT16_TO_UINT8(src[CHANNEL_L]);
    a = UINT16_TO_UINT8(src[CHANNEL_A]);
    b = UINT16_TO_UINT8(src[CHANNEL_B]);
    A = UINT16_TO_UINT8(src[CHANNEL_ALPHA]);

    double X, Y, Z, fX, fY, fZ;
    int RR, GG, BB;

    fY = pow((L + 16.0) / 116.0, 3.0);
    if (fY < 0.008856)
        fY = L / 903.3;
    Y = fY;

    if (fY > 0.008856)
        fY = pow(fY, 1.0 / 3.0);
    else
        fY = 7.787 * fY + 16.0 / 116.0;

    fX = a / 500.0 + fY;
    if (fX > 0.206893)
        X = pow(fX, 3.0);
    else
        X = (fX - 16.0 / 116.0) / 7.787;

    fZ = fY - b / 200.0;
    if (fZ > 0.206893)
        Z = pow(fZ, 3.0);
    else
        Z = (fZ - 16.0 / 116.0) / 7.787;

    X *= 0.950456 * 255;
    Y *= 255;
    Z *= 1.088754 * 255;

    RR = static_cast<int>(3.240479 * X - 1.537150 * Y - 0.498535 * Z + 0.5);
    GG = static_cast<int>(-0.969256 * X + 1.875992 * Y + 0.041556 * Z + 0.5);
    BB = static_cast<int>(0.055648 * X - 0.204043 * Y + 1.057311 * Z + 0.5);

    quint8 R = RR < 0 ? 0 : RR > 255 ? 255 : RR;
    quint8 G = GG < 0 ? 0 : GG > 255 ? 255 : GG;
    quint8 B = BB < 0 ? 0 : BB > 255 ? 255 : BB;

    c->setRgba(qRgba(R, G, B, A));
}

void KoLabColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> KoLabColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void KoLabColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *v=channelValues[1];
    *u=channelValues[2];
}

QVector <double> KoLabColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*v;
    channelValues[2]=*u;
    channelValues[3]=1.0;
    return channelValues;
}

quint8 KoLabColorSpace::scaleToU8(const quint8 *srcPixel, qint32 channelIndex) const
{
    typename ColorSpaceTraits::channels_type c = ColorSpaceTraits::nativeArray(srcPixel)[channelIndex];
    qreal b = 0;
    switch (channelIndex) {
    case ColorSpaceTraits::L_pos:
        b = ((qreal)c) / ColorSpaceTraits::math_trait::unitValueL;
        break;
    case ColorSpaceTraits::a_pos:
    case ColorSpaceTraits::b_pos:
        if (c <= ColorSpaceTraits::math_trait::halfValueAB) {
            b = ((qreal)c - ColorSpaceTraits::math_trait::zeroValueAB) / (2.0 * (ColorSpaceTraits::math_trait::halfValueAB - ColorSpaceTraits::math_trait::zeroValueAB));
        } else {
            b = 0.5 + ((qreal)c - ColorSpaceTraits::math_trait::halfValueAB) / (2.0 * (ColorSpaceTraits::math_trait::unitValueAB - ColorSpaceTraits::math_trait::halfValueAB));
        }
        break;
    default:
        b = ((qreal)c) / ColorSpaceTraits::math_trait::unitValue;
        break;
    }

    return KoColorSpaceMaths<qreal, quint8>::scaleToA(b);
}

void KoLabColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const
{
    for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
        for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
            KoChannelInfo *channel = this->channels().at(channelIndex);
            qint32 channelSize = channel->size();
            if (channel->channelType() == KoChannelInfo::COLOR) {
                if (channelIndex == ColorSpaceTraits::L_pos) {
                    ColorSpaceTraits::channels_type c = ColorSpaceTraits::parent::nativeArray((src + (pixelIndex * ColorSpaceTraits::pixelSize)))[selectedChannelIndex];
                    switch (selectedChannelIndex) {
                    case ColorSpaceTraits::L_pos:
                        break;
                    case ColorSpaceTraits::a_pos:
                    case ColorSpaceTraits::b_pos:
                        if (c <= ColorSpaceTraits::math_trait::halfValueAB) {
                            c = ColorSpaceTraits::math_trait::unitValueL * (((qreal)c - ColorSpaceTraits::math_trait::zeroValueAB) / (2.0 * (ColorSpaceTraits::math_trait::halfValueAB - ColorSpaceTraits::math_trait::zeroValueAB)));
                        } else {
                            c = ColorSpaceTraits::math_trait::unitValueL * (0.5 + ((qreal)c - ColorSpaceTraits::math_trait::halfValueAB) / (2.0 * (ColorSpaceTraits::math_trait::unitValueAB - ColorSpaceTraits::math_trait::halfValueAB)));
                        }
                        break;
                    // As per KoChannelInfo alpha channels are [0..1]
                    default:
                        c = ColorSpaceTraits::math_trait::unitValueL * (qreal)c / ColorSpaceTraits::math_trait::unitValue;
                        break;
                    }
                    ColorSpaceTraits::parent::nativeArray(dst + (pixelIndex * ColorSpaceTraits::pixelSize))[channelIndex] = c;
                } else {
                    ColorSpaceTraits::parent::nativeArray(dst + (pixelIndex * ColorSpaceTraits::pixelSize))[channelIndex] = ColorSpaceTraits::math_trait::halfValueAB;
                }
            } else if (channel->channelType() == KoChannelInfo::ALPHA) {
                memcpy(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), channelSize);
            }
        }
    }
}

void KoLabColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const
{
    for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
        for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
            KoChannelInfo *channel = this->channels().at(channelIndex);
            qint32 channelSize = channel->size();
            if (selectedChannels.testBit(channelIndex)) {
                memcpy(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), channelSize);
            } else {
                ColorSpaceTraits::channels_type v;
                switch (channelIndex) {
                case ColorSpaceTraits::L_pos:
                    v = ColorSpaceTraits::math_trait::halfValueL;
                    break;
                case ColorSpaceTraits::a_pos:
                case ColorSpaceTraits::b_pos:
                    v = ColorSpaceTraits::math_trait::halfValueAB;
                    break;
                default:
                    v = ColorSpaceTraits::math_trait::zeroValue;
                    break;
                }
                reinterpret_cast<ColorSpaceTraits::channels_type *>(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize))[0] = v;
            }
        }
    }
}
