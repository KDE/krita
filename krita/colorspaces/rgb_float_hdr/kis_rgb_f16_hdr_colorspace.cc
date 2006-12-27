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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <lcms.h>

#include <QImage>
#include <QColor>

#include <kdebug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_rgb_f16_hdr_colorspace.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

KisRgbF16HDRColorSpace::KisRgbF16HDRColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p)
: KisRgbFloatHDRColorSpace<RgbF16Traits>("RGBAF16HALF", i18n("RGB (16-bit float/channel)"), parent, RGBAF16HALF_LCMS_TYPE)
{
    m_channels.push_back(new KoChannelInfo(i18n("Red"), 2 * sizeof(half), KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, sizeof(half), QColor(255,0,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Green"), 1 * sizeof(half), KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, sizeof(half), QColor(0,255,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Blue"), 0, KoChannelInfo::COLOR, KoChannelInfo::FLOAT16, sizeof(half), QColor(0,0,255)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), 3 * sizeof(half), KoChannelInfo::ALPHA, KoChannelInfo::FLOAT16));

    m_compositeOps.insert( COMPOSITE_OVER, new KoCompositeOpOver<RgbF16Traits>( this ) );
    m_compositeOps.insert( COMPOSITE_ERASE, new KoCompositeOpErase<RgbF16Traits>( this ) );
}
