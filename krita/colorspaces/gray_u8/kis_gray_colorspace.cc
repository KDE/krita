/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include <kdebug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_gray_colorspace.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"


KisGrayColorSpace ::KisGrayColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
            KoLcmsColorSpace<GrayU8Traits>("GRAYU8", i18n("Grayscale 8-bit integer/channel)"), parent, TYPE_GRAY_8, icSigGrayData, p)
{
    addChannel(new KoChannelInfo(i18n("Gray"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8));

    init();

    addCompositeOp( new KoCompositeOpOver<GrayU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<GrayU8Traits>( this ) );
}
