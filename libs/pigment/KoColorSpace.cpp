/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoColorSpace.h"

KoColorSpace::KoColorSpace(const KoID &id, KoColorSpaceRegistry * parent)
    : m_id(id)
    , m_parent( parent )
{
    //m_dcop = 0;
}

KoColorSpace::~KoColorSpace()
{
    //delete m_dcop;
}

quint8 *KoColorSpace::allocPixelBuffer(quint32 numPixels) const
{
    return new quint8[pixelSize()*numPixels];
}

bool KoColorSpace::convertPixelsTo(const quint8 * src,
					    quint8 * dst,
					    KoColorSpace * dstColorSpace,
					    quint32 numPixels,
					    qint32 renderingIntent)
{
    // 4 channels: labA, 2 bytes per lab channel
    quint8 *pixels = new quint8[2*4*numPixels];

    toLabA16(src, pixels,numPixels);
    dstColorSpace->fromLabA16(pixels, dst,numPixels);

    delete [] pixels;

    return true;
}
