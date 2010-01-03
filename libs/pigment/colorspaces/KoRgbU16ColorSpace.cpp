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
        KoSimpleColorSpace<KoRgbU16Traits>(colorSpaceId(),
                                           i18n("RGB (16-bit integer/channel, unmanaged)"),
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
    QVector<float> channelValues;
    channelValues << c.blueF() << c.greenF() << c.redF() << c.alphaF();
    fromNormalisedChannelsValue(dst, channelValues);
}

void KoRgbU16ColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    QVector<float> channelValues(4);
    normalisedChannelsValue(src, channelValues);
    c->setRgbF(channelValues[2], channelValues[1], channelValues[0], channelValues[3]);
}
