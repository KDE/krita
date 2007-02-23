/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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

#include "kis_xyz_u16_colorspace.h"

#include <kdebug.h>
#include <klocale.h>

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

KisXyzU16ColorSpace::KisXyzU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
 KoLcmsColorSpace<XyzU16Traits>("XYZA16", i18n("XYZ (16-bit integer/channel)"), parent, TYPE_XYZA_16, icSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));
    init();
    
    addCompositeOp( new KoCompositeOpOver<XyzU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<XyzU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpMultiply<XyzU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpDivide<XyzU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpBurn<XyzU16Traits>( this ) );
}

bool KisXyzU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) 
        return true;
    else
        return false;
}

