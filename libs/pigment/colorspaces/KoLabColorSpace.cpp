/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
        KoSimpleColorSpace<KoLabU16Traits>("LABA",
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
}

void KoLabColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
}

bool KoLabColorSpace::convertPixelsTo(const quint8 *src,
                                      quint8 *dst, const KoColorSpace * dstColorSpace,
                                      quint32 numPixels,
                                      KoColorConversionTransformation::Intent /*renderingIntent*/) const
{
}

void KoLabColorSpace::toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

void KoLabColorSpace::fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

void KoLabColorSpace::toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

void KoLabColorSpace::fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

QImage KoLabColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                        const KoColorProfile * /*dstProfile*/, KoColorConversionTransformation::Intent /*renderingIntent*/) const
{
    QImage img(width, height, QImage::Format_Indexed8);
    QVector<QRgb> table;
    for (int i = 0; i < 255; ++i) table.append(qRgb(i, i, i));
    img.setColorTable(table);

    quint8* data_img = img.bits();
    for (int i = 0; i < width * height; ++i) {
        data_img[i] = data[i];
    }
    return img;
}


