/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <QImage>
#include <QColor>

#include <kdebug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>

#include "kis_rgb_colorspace.h"

#include "kis_rgb_u8_compositeop.h"

#include "compositeops/KoCompositeOps.h"

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((quint8) (257UL*(value)))

KisRgbColorSpace ::KisRgbColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
            KoLcmsColorSpace<RgbU8Traits>("RGBA", i18n("RGB 8-bit integer/channel)"), parent, TYPE_BGRA_8, icSigRgbData, p)
{
    addChannel(new KoChannelInfo(i18n("Red"), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255,0,0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0,255,0)));
    addChannel(new KoChannelInfo(i18n("Blue"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0,0,255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    init();

    addStandardCompositeOps<RgbU8Traits>(this);
    addCompositeOp( new KisRgbU8CompositeOp(this, COMPOSITE_DARKEN,  i18n( "Darken" )));
    addCompositeOp( new KisRgbU8CompositeOp(this, COMPOSITE_LIGHTEN,  i18n( "Lighten" )));
    addCompositeOp( new KisRgbU8CompositeOp(this, COMPOSITE_HUE,  i18n( "Hue" )));
    addCompositeOp( new KisRgbU8CompositeOp(this, COMPOSITE_SATURATION,  i18n( "Saturation" )));
    addCompositeOp( new KisRgbU8CompositeOp(this, COMPOSITE_VALUE,  i18n( "Value" )));
    addCompositeOp( new KisRgbU8CompositeOp(this, COMPOSITE_COLOR,  i18n( "Color" )));
}
