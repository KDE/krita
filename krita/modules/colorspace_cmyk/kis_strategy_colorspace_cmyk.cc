/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can CYANistribute it and/or modify
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
#include "kis_strategy_colorspace_cmyk.h"
#include "tiles/kispixeldata.h"
#include "kis_iterators_pixel.h"

namespace {
	const Q_INT32 MAX_CHANNEL_CMYK = 4;
	// Is it actually possible to have transparency with CMYK?
	const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}

// Init static data
ColorLUT KisStrategyColorSpaceCMYK::m_rgbLUT = ColorLUT();
ChannelInfo KisStrategyColorSpaceCMYK::channelInfo[4] = { ChannelInfo("Cyan", 4), 
							  ChannelInfo("Magenta", 3), 
							  ChannelInfo("Yellow", 2), 
							  ChannelInfo("Black", 1) };


KisStrategyColorSpaceCMYK::KisStrategyColorSpaceCMYK() : 
	KisStrategyColorSpace("CMYKA")
{
}

KisStrategyColorSpaceCMYK::~KisStrategyColorSpaceCMYK()
{
}

void KisStrategyColorSpaceCMYK::nativeColor(const KoColor& c, QUANTUM *dst)
{
	dst[PIXEL_CYAN] = upscale( c.C() );
	dst[PIXEL_MAGENTA] = upscale( c.M() );
	dst[PIXEL_YELLOW] = upscale( c.Y() );
	dst[PIXEL_BLACK] = upscale( c.K() );
}

void KisStrategyColorSpaceCMYK::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_CYAN] = upscale( c.C() );
	dst[PIXEL_MAGENTA] = upscale( c.M() );
	dst[PIXEL_YELLOW] = upscale( c.Y() );
	dst[PIXEL_BLACK] = upscale( c.K() );
	dst[PIXEL_CMYK_ALPHA] = opacity;
}

void KisStrategyColorSpaceCMYK::nativeColor(const QColor& c, QUANTUM *dst)
{
	KoColor k = KoColor( c );

	dst[PIXEL_CYAN] = upscale( k.C() );
	dst[PIXEL_MAGENTA] = upscale( k.M() );
	dst[PIXEL_YELLOW] = upscale( k.Y() );
	dst[PIXEL_BLACK] = upscale( k.K() );
}

void KisStrategyColorSpaceCMYK::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{
	KoColor k = KoColor( c );

	dst[PIXEL_CYAN] = upscale( k.C() );
	dst[PIXEL_MAGENTA] = upscale( k.M() );
	dst[PIXEL_YELLOW] = upscale( k.Y() );
	dst[PIXEL_BLACK] = upscale( k.K() );
	dst[PIXEL_CMYK_ALPHA] = opacity;

}

void KisStrategyColorSpaceCMYK::nativeColor(QRgb rgb, QUANTUM *dst)
{
	KoColor k = KoColor(QColor( rgb ));

	dst[PIXEL_CYAN] = upscale( k.C() );
	dst[PIXEL_MAGENTA] = upscale( k.M() );
	dst[PIXEL_YELLOW] = upscale( k.Y() );
	dst[PIXEL_BLACK] = upscale( k.K() );
}

void KisStrategyColorSpaceCMYK::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{
	KoColor k = KoColor(QColor( rgb ));

	dst[PIXEL_CYAN] = upscale( k.C() );
	dst[PIXEL_MAGENTA] = upscale( k.M() );
	dst[PIXEL_YELLOW] = upscale( k.Y() );
	dst[PIXEL_BLACK] = upscale( k.K() );
	dst[PIXEL_CMYK_ALPHA] = opacity;
}

void KisStrategyColorSpaceCMYK::toKoColor(const QUANTUM *src, KoColor *c)
{
	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
}

void KisStrategyColorSpaceCMYK::toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity)
{
	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
	*opacity = src[PIXEL_CMYK_ALPHA];
}

ChannelInfo* KisStrategyColorSpaceCMYK::channelsInfo() const
{
	return channelInfo;
}
bool KisStrategyColorSpaceCMYK::alpha() const
{
	return true;
}
Q_INT32 KisStrategyColorSpaceCMYK::depth() const
{
	return MAX_CHANNEL_CMYKA;
}

QImage KisStrategyColorSpaceCMYK::convertToImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const 
{
	QImage img(width, height, 32, 0, QImage::LittleEndian);
	Q_INT32 i = 0;
	uchar *j = img.bits();

	while ( i < stride * height ) {
		
		RGB r;
		// Check in LUT whether k already exists; if so, grab it, else
		CMYK c;
		c.c = *( data + i + PIXEL_CYAN );
		c.m = *( data + i + PIXEL_MAGENTA );
		c.y = *( data + i + PIXEL_YELLOW );
		c.k = *( data + i + PIXEL_BLACK );
		
		if ( m_rgbLUT.contains ( c ) ) {
			r =  m_rgbLUT[c];
		}
		else {
			// Accessing the rgba of KoColor automatically converts
			// from cmyk to rgb and caches the result.
			KoColor k = KoColor(c.c,
					    c.m,
					    c.y,
					    c.k );
			// Store as little as possible
			r.r =  k.R();
			r.g =  k.G();
			r.b =  k.B();
			m_rgbLUT[c] = r;
		}
		
		// fix the pixel in QImage.
		*( j + PIXEL_ALPHA ) = *( data + i + PIXEL_CMYK_ALPHA );
		*( j + PIXEL_RED )   = r.r;
		*( j + PIXEL_GREEN ) =  r.g;
		*( j + PIXEL_BLUE )  =  r.b;
		
		i += MAX_CHANNEL_CMYKA;
		j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
		
	}

	return img;
}

// void KisStrategyColorSpaceCMYK::bitBlt(Q_INT32 stride,
// 				       QUANTUM *dst,
// 				       Q_INT32 dststride,
// 				       QUANTUM *src,
// 				       Q_INT32 srcstride,
// 				       Q_INT32 rows,
// 				       Q_INT32 cols,
// 				       CompositeOp op) const
// {
//         kdDebug() << "Compositing with: " << op << endl;
// 	Q_INT32 linesize = stride * sizeof(QUANTUM) * cols;
// 	QUANTUM *d;
// 	QUANTUM *s;

// 	if (rows <= 0 || cols <= 0)
// 		return;

// 	switch (op) {
// 	case COMPOSITE_COPY:

// 	case COMPOSITE_CLEAR:

// 	case COMPOSITE_OVER:
// 		bitBlt(stride, dst, dststride, src, srcstride, OPACITY_OPAQUE, rows, cols, op);
// 		break;
// 	default:
// 		kdDebug() << "Not Implemented.\n";
// 		abort();
// 		return;
// 	}
// }

void KisStrategyColorSpaceCMYK::bitBlt(Q_INT32 stride,
				       QUANTUM *dst,
				       Q_INT32 dststride,
				       KisStrategyColorSpaceSP srcSpace,
				       QUANTUM *src,
				       Q_INT32 srcstride,
				       QUANTUM opacity,
				       Q_INT32 rows,
				       Q_INT32 cols,
				       CompositeOp op) const
{
	Q_INT32 linesize = stride * sizeof(QUANTUM) * cols;
	QUANTUM alpha = OPACITY_OPAQUE;
	QUANTUM *d;
	QUANTUM *s;
	Q_INT32 i;

	if (rows <= 0 || cols <= 0)
		return;
	switch (op) {
	case COMPOSITE_COPY:
		d = dst;
		s = src;

		while (rows-- > 0) {
			memcpy(d, s, linesize);
			d += dststride;
			s += srcstride;
		}
		break;
	case COMPOSITE_CLEAR:
		d = dst;
		s = src;
		while (rows-- > 0) {
			memset(d, 0, linesize);
			d += dststride;
		}
		break;
	case COMPOSITE_OVER:
	default:
		while (rows-- > 0) {
			d = dst;
			s = src;
			for (i = cols; i > 0; i--, d += stride, s += stride) {
				// XXX: this is probably incorrect. CMYK simulates ink; layers of
				// ink over ink until a certain maximum, QUANTUM_MAX arbitrarily
				// is reached. We invert the opacity because opacity_transparent
				// and opacity_opaque were designed for RGB, which works the other
				// way around.
				alpha = (QUANTUM_MAX - opacity) + (QUANTUM_MAX - alpha);
				if (alpha >= OPACITY_OPAQUE) // OPAQUE is CMYK transparent
					continue;
				if (s[PIXEL_CYAN] > alpha) {
					d[PIXEL_CYAN] = d[PIXEL_CYAN] + (s[PIXEL_CYAN] - alpha);
				}
				if (s[PIXEL_MAGENTA] > alpha) {
					d[PIXEL_MAGENTA] = d[PIXEL_MAGENTA] + (s[PIXEL_MAGENTA] - alpha);
				}
				if (s[PIXEL_YELLOW] > alpha) {
					d[PIXEL_YELLOW] = d[PIXEL_YELLOW]  + (s[PIXEL_YELLOW] - alpha);
				}
				if (s[PIXEL_BLACK] > alpha) {
					d[PIXEL_BLACK] = d[PIXEL_BLACK] + (s[PIXEL_BLACK] - alpha);
				}
				d[PIXEL_ALPHA] = alpha; // XXX: this is certainly incorrect.
			}
			dst += dststride;
			src += srcstride;
		}
	}

}

void KisStrategyColorSpaceCMYK::computeDuplicatePixel(KisIteratorPixel* dst, KisIteratorPixel* dab, KisIteratorPixel* src)
{
	KisPixelRepresentationCMYK dstPR(*dst);
	KisPixelRepresentationCMYK dabPR(*dab);
	KisPixelRepresentationCMYK srcPR(*src);
	dstPR.cyan() = ( (QUANTUM_MAX - dabPR.cyan()) * (srcPR.cyan()) ) / QUANTUM_MAX;
	dstPR.magenta() = ( (QUANTUM_MAX - dabPR.magenta()) * (srcPR.magenta()) ) / QUANTUM_MAX;
	dstPR.yellow() = ( (QUANTUM_MAX - dabPR.yellow()) * (srcPR.yellow()) ) / QUANTUM_MAX;
	dstPR.black() = ( (QUANTUM_MAX - dabPR.black()) * (srcPR.black()) ) / QUANTUM_MAX;
	dstPR.alpha() =( dabPR.alpha() * (srcPR.alpha()) ) / QUANTUM_MAX;
}

// XXX: these algorithms aren't the best. See www.littlecms.com
// for a suitable library, or the posting by Leo Rosenthol for
// a better, but slower algorithm at
// http://lists.kde.org/?l=koffice-devel&m=106698241227054&w=2

// XXX: (bsar) No need to implement bad algorithms again when we have 
// the same bad implementation already in koColor.
void KisStrategyColorSpaceCMYK::convertToRGBA(KisPixelRepresentation& src, KisPixelRepresentationRGB& dst)
{
	KisPixelRepresentationCMYK srccmyk(src);
	dst.red() = QUANTUM_MAX - (srccmyk.cyan() + srccmyk.black());
	dst.green() = QUANTUM_MAX - (srccmyk.magenta() + srccmyk.black());
	dst.blue() = QUANTUM_MAX - (srccmyk.yellow() + srccmyk.black());
}
void KisStrategyColorSpaceCMYK::convertFromRGBA(KisPixelRepresentationRGB& src, KisPixelRepresentation& dst)
{
	KisPixelRepresentationCMYK dstcmyk(dst);
	dstcmyk.cyan() = QUANTUM_MAX - src.red();
	dstcmyk.magenta() = QUANTUM_MAX - src.green();
	dstcmyk.yellow() = QUANTUM_MAX - src.blue();
	dstcmyk.black() = QMIN(dstcmyk.cyan(), QMIN( dstcmyk.magenta(), dstcmyk.yellow() ) );
	dstcmyk.cyan() -= dstcmyk.black();
	dstcmyk.magenta() -= dstcmyk.black();
	dstcmyk.yellow() -= dstcmyk.black();
}
