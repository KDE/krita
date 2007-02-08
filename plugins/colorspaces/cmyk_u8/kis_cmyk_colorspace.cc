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

#include "kis_cmyk_colorspace.h"

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"

KisCmykU8ColorSpace::KisCmykU8ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
 KoLcmsColorSpace<CmykU8Traits>("CMYK", i18n("CMYK (8-bit integer/channel)"), parent, TYPE_CMYK5_8, icSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint8), KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));
    init();
    
    addCompositeOp( new KoCompositeOpOver<CmykU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<CmykU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpMultiply<CmykU8Traits>( this ) );
}

bool KisCmykU8ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) 
        return true;
    else
        return false;
}

