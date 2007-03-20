/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_ycbcr_u16_colorspace.h"

#include <QImage>
#include <QColor>

#include <kdebug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

KisYCbCrU16ColorSpace::KisYCbCrU16ColorSpace(KoColorSpaceRegistry * parent, KoColorProfile */*p*/)
: KisYCbCrBaseColorSpace<YCbCrU16Traits>("YCbCrAU16", i18n("YCbCr (16-bit integer/channel)"), parent, TYPE_YCbCr_16)
{
    addChannel(new KoChannelInfo(i18n("Y"), YCbCrU16Traits::y_pos * sizeof(Q_UINT16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(Q_UINT16), QColor(255,0,0)));
    addChannel(new KoChannelInfo(i18n("Cb"), YCbCrU16Traits::cb_pos * sizeof(Q_UINT16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(Q_UINT16), QColor(0,255,0)));
    addChannel(new KoChannelInfo(i18n("Cr"), YCbCrU16Traits::cr_pos * sizeof(Q_UINT16), KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(Q_UINT16), QColor(0,0,255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), YCbCrU16Traits::alpha_pos * sizeof(Q_UINT16), KoChannelInfo::ALPHA, KoChannelInfo::UINT16));

    addCompositeOp( new KoCompositeOpOver<YCbCrU16Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<YCbCrU16Traits>( this ) );
}
