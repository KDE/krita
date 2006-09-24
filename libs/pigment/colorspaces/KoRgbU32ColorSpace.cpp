/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoRgbU32ColorSpace.h"

#include <kdebug.h>
#include <klocale.h>

#include "KoCompositeOp.h"

KoRgbU32ColorSpace::KoRgbU32ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
 KoLcmsColorSpace<RgbU32Traits>("RGBU32", i18n("RGB 32-bit integer/channel)"), parent, TYPE_BGRA_32, icSigRgbData, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Red"), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT32, 4, QColor(255,0,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Green"), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT32, 4, QColor(0,255,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Blue"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT32, 4, QColor(0,0,255)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));
    init();
//     m_compositeOps.insert( COMPOSITE_OVER, new CompositeOver( this ) );
//     m_compositeOps.insert( COMPOSITE_ERASE, new CompositeErase( this ) );
}

bool KoRgbU32ColorSpace::willDegrade(ColorSpaceIndependence independence)
{
    if (independence == TO_RGBA8) 
        return true;
    else
        return false;
}
