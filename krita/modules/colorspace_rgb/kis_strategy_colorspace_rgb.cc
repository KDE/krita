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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_rgb.h"
#include "tiles/kispixeldata.h"
#include "composite.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() :
	KisStrategyColorSpace("RGBA", TYPE_BGRA_8, icSigRgbData)
{
	setProfile(cmsCreate_sRGBProfile());

	m_channels.push_back(new KisChannelInfo(i18n("red"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("green"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("blue"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("alpha"), 3, ALPHA));
	
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
}

void KisStrategyColorSpaceRGB::nativeColor(const KoColor& c, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.R());
	dst[PIXEL_GREEN] = upscale(c.G());
	dst[PIXEL_BLUE] = upscale(c.B());
}

void KisStrategyColorSpaceRGB::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.R());
	dst[PIXEL_GREEN] = upscale(c.G());
	dst[PIXEL_BLUE] = upscale(c.B());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::toKoColor(const QUANTUM *src, KoColor *c)
{
	c -> setRGB(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE]));
}

void KisStrategyColorSpaceRGB::toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity)
{
	c -> setRGB(downscale(src[PIXEL_RED]), downscale(src[PIXEL_GREEN]), downscale(src[PIXEL_BLUE]));
	*opacity = src[PIXEL_ALPHA];
}

vKisChannelInfoSP KisStrategyColorSpaceRGB::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceRGB::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceRGB::depth() const
{
	return MAX_CHANNEL_RGBA;
}

Q_INT32 KisStrategyColorSpaceRGB::nColorChannels() const
{
	return MAX_CHANNEL_RGB;
}

QImage KisStrategyColorSpaceRGB::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride)
{
	QImage img;
	
#ifdef __BIG_ENDIAN__
	img = QImage(width, height, 32, 0, QImage::LittleEndian);
	Q_INT32 i = 0;
	uchar *j = img.bits();
	
	while ( i < stride * height ) {

		// Swap the bytes
		*( j + 0)  = *( data + i + PIXEL_ALPHA );
		*( j + 1 ) = *( data + i + PIXEL_RED );
		*( j + 2 ) = *( data + i + PIXEL_GREEN );
		*( j + 3 ) = *( data + i + PIXEL_BLUE );
		
		i += MAX_CHANNEL_RGBA;
		
		j += MAX_CHANNEL_RGBA; // Because we're hard-coded 32 bits deep, 4 bytes
		
	}
	
#else
	(void)stride; // Kill warning

	img = QImage(const_cast<QUANTUM *>(data), width, height, 32, 0, 0, QImage::LittleEndian);

	// XXX: The previous version of this code used the quantum data directly
	// as an optimisation. We're introducing a copy overhead here which could
	// be factored out again if needed.
	img = img.copy();
#endif
	return img;
}

void KisStrategyColorSpaceRGB::bitBlt(Q_INT32 stride,
				      QUANTUM *dst, 
				      Q_INT32 dststride,
				      QUANTUM *src, 
				      Q_INT32 srcstride,
				      QUANTUM opacity,
				      Q_INT32 rows, 
				      Q_INT32 cols, 
				      CompositeOp op)
{

	switch (op) {
	case COMPOSITE_UNDEF:
		// Undefined == no composition
		break;
	case COMPOSITE_OVER:
		compositeOver(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_IN:
		compositeIn(stride, dst, dststride, src, srcstride, rows, cols, opacity);
	case COMPOSITE_OUT:
		compositeOut(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_ATOP:
		compositeAtop(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_XOR:
		compositeXor(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_PLUS:
		compositePlus(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_MINUS:
		compositeMinus(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_ADD:
		compositeAdd(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_SUBTRACT:
		compositeSubtract(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_DIFF:
		compositeDiff(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_MULT:
		compositeMult(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_BUMPMAP:
		compositeBumpmap(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY:
		compositeCopy(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_RED:
		compositeCopyRed(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_GREEN:
		compositeCopyGreen(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_BLUE:
		compositeCopyBlue(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_OPACITY:
		compositeCopyOpacity(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_CLEAR:
		compositeClear(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
#if 0
	case COMPOSITE_DISSOLVE:
		compositeDissolve(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_DISPLACE:
		compositeDisplace(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_MODULATE:
		compositeModulate(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_THRESHOLD:
		compositeThreshold(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_NO:
		// No composition.
		break;
	case COMPOSITE_DARKEN:
		compositeDarken(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_LIGHTEN:
		compositeLighten(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_HUE:
		compositeHue(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_SATURATE:
		compositeSaturate(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COLORIZE:
		compositeColorize(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_LUMINIZE:
		compositeLuminize(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_SCREEN:
		compositeScreen(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_OVERLAY:
		compositeOverlay(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
#endif
	case COMPOSITE_ERASE:
		compositeErase(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	default:
		break;
	}
}

