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

#include <lcms.h>

#include <kdebug.h>
#include <klocale.h>

#include "KoColorSpaceRegistry.h"

#include "kis_rgb_f32_hdr_colorspace.h"

KisRgbF32HDRColorSpace::KisRgbF32HDRColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p)
: KisRgbFloatHDRColorSpace<RgbF32Traits>(colorSpaceId(), i18n("RGB (32-bit float/channel) for High Dynamic Range imaging"), parent, p)
{
}

QString KisRgbF32HDRColorSpace::colorSpaceId()
{
    return QString("RGBAF32");
}

