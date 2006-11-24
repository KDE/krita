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

#include "kis_cmyk_u16_colorspace.h"

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"


KisCmykU16ColorSpace::KisCmykU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
 KoLcmsColorSpace<CmykU16Traits>("CMYKA16", i18n("CMYK (16-bit integer/channel)"), parent, TYPE_CMYK5_16, icSigCmykData, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    m_channels.push_back(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    m_channels.push_back(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    m_channels.push_back(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::black));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));
    init();
    m_compositeOps.insert( COMPOSITE_OVER, new KoCompositeOpOver<CmykU16Traits>( this ) );
    m_compositeOps.insert( COMPOSITE_ERASE, new KoCompositeOpErase<CmykU16Traits>( this ) );
}

bool KisCmykU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) 
        return true;
    else
        return false;
}

