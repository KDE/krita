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

#include "KoRgbU16ColorSpace.h"

#include <limits.h>
#include <stdlib.h>

#include <QImage>
#include <QBitArray>

#include <klocale.h>

#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"
#include "KoCompositeOpOver.h"
#include "KoCompositeOpErase.h"
#include "KoCompositeOpAlphaDarken.h"


KoRgbU16ColorSpace::KoRgbU16ColorSpace() :
    KoSimpleColorSpace<KoRgbU16Traits>("ALPHA",
                                       i18n("Alpha mask"),
                                       RGBAColorModelID,
                                       Integer16BitsColorDepthID)
{
}

KoRgbU16ColorSpace::~KoRgbU16ColorSpace()
{
}


QString KoRgbU16ColorSpace::colorSpaceId()
{
    return QString("RGBA16");
}

KoColorSpace* KoRgbU16ColorSpace::clone() const
{
    return new KoRgbU16ColorSpace();
}

void KoRgbU16ColorSpace::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
}

void KoRgbU16ColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
}

bool KoRgbU16ColorSpace::convertPixelsTo(const quint8 *src,
                     quint8 *dst, const KoColorSpace * dstColorSpace,
                     quint32 numPixels,
                     KoColorConversionTransformation::Intent /*renderingIntent*/) const
{
}

void KoRgbU16ColorSpace::toLabA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

void KoRgbU16ColorSpace::fromLabA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

void KoRgbU16ColorSpace::toRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

void KoRgbU16ColorSpace::fromRgbA16(const quint8* src, quint8* dst, quint32 nPixels) const
{

}

QImage KoRgbU16ColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile * /*dstProfile*/, KoColorConversionTransformation::Intent /*renderingIntent*/) const
{
    QImage img(width, height, QImage::Format_Indexed8);
    QVector<QRgb> table;
    for(int i = 0; i < 255; ++i) table.append(qRgb(i,i,i));
    img.setColorTable(table);

    quint8* data_img = img.bits();
    for( int i = 0; i < width * height; ++i)
    {
        data_img[i] = data[i];
    }
    return img;
}

