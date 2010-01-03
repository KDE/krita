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
#include "KoRgbU8ColorSpace.h"

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
#include "compositeops/KoCompositeOps.h"
#include "compositeops/KoCompositeOpAdd.h"
#include "compositeops/KoCompositeOpSubtract.h"

KoRgbU8ColorSpace::KoRgbU8ColorSpace() :

        KoSimpleColorSpace<KoRgbU8Traits>(colorSpaceId(),
                                          i18n("RGB (8-bit integer/channel, unmanaged)"),
                                          RGBAColorModelID,
                                          Integer8BitsColorDepthID)
{
    addChannel(new KoChannelInfo(i18n("Red"),   2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255, 0, 0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 255, 0)));
    addChannel(new KoChannelInfo(i18n("Blue"),  0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 0, 255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<KoRgbU8Traits>(this);

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
