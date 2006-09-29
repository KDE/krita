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

#include "kis_rgb_colorspace.h"

#include "kis_rgb_u8_compositeop.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"


#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((quint8) (257UL*(value)))

KisRgbColorSpace ::KisRgbColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p) :
            KoLcmsColorSpace<RgbU8Traits>("RGBU8", i18n("RGB 8-bit integer/channel)"), parent, TYPE_BGRA_8, icSigRgbData, p)
{
    m_channels.push_back(new KoChannelInfo(i18n("Red"), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255,0,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Green"), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0,255,0)));
    m_channels.push_back(new KoChannelInfo(i18n("Blue"), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0,0,255)));
    m_channels.push_back(new KoChannelInfo(i18n("Alpha"), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    init();

    m_compositeOps.insert( COMPOSITE_OVER, new KoCompositeOpOver<RgbU8Traits>( this ) );
    m_compositeOps.insert( COMPOSITE_ERASE, new KoCompositeOpErase<RgbU8Traits>( this ) );
    m_compositeOps.insert(COMPOSITE_MULT, new KisRgbU8CompositeOp(this, COMPOSITE_MULT,  i18n( "Multiply" )) );
    m_compositeOps.insert(COMPOSITE_BURN, new KisRgbU8CompositeOp(this, COMPOSITE_BURN,  i18n( "Burn" )) );
    m_compositeOps.insert(COMPOSITE_DODGE, new KisRgbU8CompositeOp(this, COMPOSITE_DODGE,  i18n( "Dodge" )));
    m_compositeOps.insert(COMPOSITE_DIVIDE, new KisRgbU8CompositeOp(this, COMPOSITE_DIVIDE,  i18n( "Divide" )));
    m_compositeOps.insert(COMPOSITE_SCREEN, new KisRgbU8CompositeOp(this, COMPOSITE_SCREEN,  i18n( "Screen" )));
    m_compositeOps.insert(COMPOSITE_OVERLAY, new KisRgbU8CompositeOp(this, COMPOSITE_OVERLAY,  i18n( "Overlay" )));
    m_compositeOps.insert(COMPOSITE_DARKEN, new KisRgbU8CompositeOp(this, COMPOSITE_DARKEN,  i18n( "Darken" )));
    m_compositeOps.insert(COMPOSITE_LIGHTEN, new KisRgbU8CompositeOp(this, COMPOSITE_LIGHTEN,  i18n( "Lighten" )));
    m_compositeOps.insert(COMPOSITE_HUE, new KisRgbU8CompositeOp(this, COMPOSITE_HUE,  i18n( "Hue" )));
    m_compositeOps.insert(COMPOSITE_SATURATION, new KisRgbU8CompositeOp(this, COMPOSITE_SATURATION,  i18n( "Saturation" )));
    m_compositeOps.insert(COMPOSITE_VALUE, new KisRgbU8CompositeOp(this, COMPOSITE_VALUE,  i18n( "Value" )));
    m_compositeOps.insert(COMPOSITE_COLOR, new KisRgbU8CompositeOp(this, COMPOSITE_COLOR,  i18n( "Color" )));
}
