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

#include <limits.h>
#include <stdlib.h>

#include <qimage.h>

#include <kdebug.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_rgb.h"
#include "tiles/kispixeldata.h"
#include "composite.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

// XXX: Why no alpha channel?
ChannelInfo KisStrategyColorSpaceRGB::channelInfo[3] = { ChannelInfo("Red", 2), ChannelInfo("Green", 1), ChannelInfo("Blue", 0) };


KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() :
	KisStrategyColorSpace("RGBA")
{
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
	kdDebug() << "KisStrategyColorSpaceRGB has been destroyed" << endl;
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

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::nativeColor(QRgb rgb, QUANTUM *dst)
{
	dst[PIXEL_RED] = qRed(rgb);
	dst[PIXEL_GREEN] = qGreen(rgb);
	dst[PIXEL_BLUE] = qBlue(rgb);
}

void KisStrategyColorSpaceRGB::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = qRed(rgb);
	dst[PIXEL_GREEN] = qGreen(rgb);
	dst[PIXEL_BLUE] = qBlue(rgb);
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

ChannelInfo* KisStrategyColorSpaceRGB::channelsInfo() const
{
	return channelInfo;
}

bool KisStrategyColorSpaceRGB::alpha() const
{
	return true;
}

Q_INT32 KisStrategyColorSpaceRGB::depth() const
{
	return MAX_CHANNEL_RGBA;
}

QImage KisStrategyColorSpaceRGB::convertToImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const 
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
	case COMPOSITE_COPY_CYAN:
		compositeCopyCyan(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_MAGENTA:
		compositeCopyMagenta(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_YELLOW:
		compositeCopyYellow(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_COPY_BLACK:
		compositeCopyBlack(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	case COMPOSITE_ERASE:
		compositeErase(stride, dst, dststride, src, srcstride, rows, cols, opacity);
		break;
	default:
		kdDebug() << "Composite op " << op << " not Implemented yet.\n";
	}
}

void KisStrategyColorSpaceRGB::computeDuplicatePixel(KisIteratorPixel* dst, KisIteratorPixel* dab, KisIteratorPixel* src)
{
	KisPixelRepresentationRGB dstPR(*dst);
	KisPixelRepresentationRGB dabPR(*dab);
	KisPixelRepresentationRGB srcPR(*src);
	dstPR.red() = ( (QUANTUM_MAX - dabPR.red()) * (srcPR.red()) ) / QUANTUM_MAX;
	dstPR.green() = ( (QUANTUM_MAX - dabPR.green()) * (srcPR.green()) ) / QUANTUM_MAX;
	dstPR.blue() = ( (QUANTUM_MAX - dabPR.blue()) * (srcPR.blue()) ) / QUANTUM_MAX;
	dstPR.alpha() =( dabPR.alpha() * (srcPR.alpha()) ) / QUANTUM_MAX;
}

void KisStrategyColorSpaceRGB::convertToRGBA(KisPixelRepresentation& src, KisPixelRepresentationRGB& dst)
{
	for(int i = 0; i < MAX_CHANNEL_RGBA; i++)
	{
		dst[i] = src[i];
	}
}

void KisStrategyColorSpaceRGB::convertFromRGBA(KisPixelRepresentationRGB& src, KisPixelRepresentation& dst)
{
	for(int i = 0; i < MAX_CHANNEL_RGBA; i++)
	{
		dst[i] = src[i];
	}
}
