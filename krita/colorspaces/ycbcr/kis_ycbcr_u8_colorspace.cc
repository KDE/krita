/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_ycbcr_u8_colorspace.h"
#include <QImage>
#include <QColor>

#include <kis_debug.h>
#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionTransformationFactory.h>
#include <KoScaleColorConversionTransformation.h>
#include "kis_ycbcr_to_rgb_color_conversion_transformation.h"
#include "kis_rgb_to_ycbcr_color_conversion_transformation.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"

KisYCbCrU8ColorSpace::KisYCbCrU8ColorSpace( const KoCtlColorProfile *p)
: KisYCbCrBaseColorSpace<YCbCrU8Traits>("YCbCrAU8", i18n("YCbCr (8-bit integer/channel)"), p)
{
    addChannel(new KoChannelInfo(i18n("Y"), YCbCrU8Traits::y_pos * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(255,0,0)));
    addChannel(new KoChannelInfo(i18n("Cb"), YCbCrU8Traits::cb_pos * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(0,255,0)));
    addChannel(new KoChannelInfo(i18n("Cr"), YCbCrU8Traits::cr_pos * sizeof(quint8), KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(0,0,255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), YCbCrU8Traits::alpha_pos * sizeof(quint8), KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    addCompositeOp( new KoCompositeOpOver<YCbCrU8Traits>( this ) );
    addCompositeOp( new KoCompositeOpErase<YCbCrU8Traits>( this ) );
}

KoColorSpace* KisYCbCrU8ColorSpace::clone() const
{
    return new KisYCbCrU8ColorSpace( static_cast<const KoCtlColorProfile*>(profile()));
}

