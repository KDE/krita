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
#include <qpainter.h>
#include <qpixmap.h>

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

ChannelInfo KisStrategyColorSpaceRGB::channelInfo[3] = { ChannelInfo("Red", 2), ChannelInfo("Green", 1), ChannelInfo("Blue", 0) };


KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() :
	KisStrategyColorSpace("RGBA"),
	m_pixmap(RENDER_WIDTH * 2, RENDER_HEIGHT * 2)
{
	m_buf = new QUANTUM[RENDER_WIDTH * RENDER_HEIGHT * MAX_CHANNEL_RGBA];
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
	kdDebug() << "KisStrategyColorSpaceRGB has been destroyed" << endl;
	delete[] m_buf;
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


void KisStrategyColorSpaceRGB::render(KisImageSP image, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	QImage img = convertToImage(image, x, y, width, height);
	if (!img.isNull()) {
		m_pixio.putImage(&m_pixmap, 0, 0, &img);
		painter.drawPixmap(x, y, m_pixmap, 0, 0, width, height);	
	}
}


QImage KisStrategyColorSpaceRGB::convertToImage(KisImageSP image, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const 
{
	if (!image) return QImage();

	return convertToImage(image -> tiles(), image -> depth(), x, y, width, height);
}

QImage KisStrategyColorSpaceRGB::convertToImage(KisTileMgrSP tm, Q_UINT32 depth, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height) const 
{
	if (!tm) return QImage();

	KisPixelDataSP pd = new KisPixelData;
	QImage img;
	
	pd -> mgr = 0;
	pd -> tile = 0;
	pd -> mode = TILEMODE_READ;
	pd -> x1 = x;
	pd -> x2 = x + width - 1;
	pd -> y1 = y;
	pd -> y2 = y + height - 1;
	pd -> width = pd -> x2 - pd -> x1 + 1;
	pd -> height = pd -> y2 - pd -> y1 + 1;
	pd -> depth = depth;
	pd -> stride = pd -> depth * pd -> width;
	pd -> owner = false;
	pd -> data = m_buf;
	tm -> readPixelData(pd);
	
#ifdef __BIG_ENDIAN__
	img = QImage(pd->width,  pd->height, 32, 0, QImage::LittleEndian);
	Q_INT32 i = 0;
	uchar *j = img.bits();
	
	while ( i < pd ->stride * pd -> height ) {
		
		// Swap the bytes
		*( j + 0)  = *( pd->data + i + PIXEL_ALPHA );
		*( j + 1 ) = *( pd->data + i + PIXEL_RED );
		*( j + 2 ) = *( pd->data + i + PIXEL_GREEN );
		*( j + 3 ) = *( pd->data + i + PIXEL_BLUE );
		
		i += MAX_CHANNEL_RGBA;
		j += MAX_CHANNEL_RGBA; // Because we're hard-coded 32 bits deep, 4 bytes
		
	}
	
#else
	img = QImage(pd -> data, pd -> width, pd -> height, pd -> depth * QUANTUM_DEPTH, 0, 0, QImage::LittleEndian);
#endif
	return img;

}


void KisStrategyColorSpaceRGB::bitBlt(Q_INT32 stride,
				      QUANTUM *dst,
				      Q_INT32 dststride,
				      QUANTUM *src,
				      Q_INT32 srcstride,
				      Q_INT32 rows, 
				      Q_INT32 cols, 
				      CompositeOp op) const
{
	if (rows <= 0 || cols <= 0)
		return;

	bitBlt(stride, dst, dststride, src, srcstride, OPACITY_OPAQUE, rows, cols, op);
}

void KisStrategyColorSpaceRGB::bitBlt(Q_INT32 stride,
				      QUANTUM *dst, 
				      Q_INT32 dststride,
				      QUANTUM *src, 
				      Q_INT32 srcstride,
				      QUANTUM opacity,
				      Q_INT32 rows, 
				      Q_INT32 cols, 
				      CompositeOp op) const
{
	if (rows <= 0 || cols <= 0)
		return;


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
		return;
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


