/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_colorspace.h"
#include "kis_colorspace_iface.h"

KisColorSpace::KisColorSpace(const KisID &id, KisColorSpaceFactoryRegistry * parent)
    : m_id(id)
    , m_parent( parent )
{
    m_dcop = 0;
}

KisColorSpace::~KisColorSpace()
{
    delete m_dcop;
}

DCOPObject * KisColorSpace::dcopObject()
{
    if (!m_dcop) {
        m_dcop = new KisColorSpaceIface(this);
        Q_CHECK_PTR(m_dcop);
    }
    return m_dcop;
}

quint8 *KisColorSpace::allocPixelBuffer(quint32 numPixels) const
{
    return new quint8[pixelSize()*numPixels];
}

bool KisColorSpace::convertPixelsTo(const quint8 * src,
					    quint8 * dst,
					    KisColorSpace * dstColorSpace,
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
