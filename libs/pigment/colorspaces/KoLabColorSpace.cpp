/*
 *  Copyright (c) 2004-2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KoLabColorSpace.h"

#include <limits.h>
#include <stdlib.h>

#include <QImage>
#include <QBitArray>

#include <klocale.h>

#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"

#include "../compositeops/KoCompositeOpOver.h"
#include "../compositeops/KoCompositeOpErase.h"
#include "../compositeops/KoCompositeOpMultiply.h"
#include "../compositeops/KoCompositeOpDivide.h"
#include "../compositeops/KoCompositeOpBurn.h"

KoLabColorSpace::KoLabColorSpace() :
        KoSimpleColorSpace<KoLabU16Traits>(colorSpaceId(),
                                           i18n("L*a*b* (16-bit integer/channel, unmanaged)"),
                                           LABAColorModelID,
                                           Integer16BitsColorDepthID)
{
    addChannel(new KoChannelInfo(i18n("Lightness"), CHANNEL_L     * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(100, 100, 100)));
    addChannel(new KoChannelInfo(i18n("a*"),        CHANNEL_A     * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(150, 150, 150)));
    addChannel(new KoChannelInfo(i18n("b*"),        CHANNEL_B     * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), QColor(200, 200, 200)));
    addChannel(new KoChannelInfo(i18n("Alpha"),     CHANNEL_ALPHA * sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    addCompositeOp(new KoCompositeOpOver<KoLabU16Traits>(this));
    addCompositeOp(new KoCompositeOpErase<KoLabU16Traits>(this));
    addCompositeOp(new KoCompositeOpMultiply<KoLabU16Traits>(this));
    addCompositeOp(new KoCompositeOpDivide<KoLabU16Traits>(this));
    addCompositeOp(new KoCompositeOpBurn<KoLabU16Traits>(this));

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
