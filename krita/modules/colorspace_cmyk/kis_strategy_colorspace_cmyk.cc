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
#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_config.h"
#include "kis_image.h"
#include "kis_strategy_colorspace_cmyk.h"
#include "kis_colorspace_registry.h"
#include "tiles/kispixeldata.h"
#include "kis_iterators_pixel.h"

#include "kis_resource.h"
#include "kis_resourceserver.h"
#include "kis_resource_mediator.h"
#include "kis_factory.h"
#include "kis_profile.h"

namespace {
	const Q_INT32 MAX_CHANNEL_CMYK = 4;
}

KisStrategyColorSpaceCMYK::KisStrategyColorSpaceCMYK() :
	KisStrategyColorSpace("CMYK", TYPE_CMYK_8, icSigCmykData)
{
	m_channels.push_back(new KisChannelInfo(i18n("cyan"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("magenta"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("yellow"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("black"), 3, COLOR));
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
}

void KisStrategyColorSpaceCMYK::toKoColor(const QUANTUM *src, KoColor *c)
{
	c -> setCMYK(downscale(src[PIXEL_CYAN]), 
		     downscale(src[PIXEL_MAGENTA]), 
		     downscale(src[PIXEL_YELLOW]), 
		     downscale(src[PIXEL_BLACK]));
}

void KisStrategyColorSpaceCMYK::toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity)
{
	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
	*opacity = OPACITY_OPAQUE;
}

vKisChannelInfoSP KisStrategyColorSpaceCMYK::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceCMYK::alpha() const
{
	return false;
}

Q_INT32 KisStrategyColorSpaceCMYK::depth() const
{
	return MAX_CHANNEL_CMYK;
}

Q_INT32 KisStrategyColorSpaceCMYK::nColorChannels() const
{
	return MAX_CHANNEL_CMYK;
}

QImage KisStrategyColorSpaceCMYK::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, 
						  KisProfileSP srcProfile, KisProfileSP dstProfile, 
						  Q_INT32 renderingIntent)

{
	kdDebug() << "convertToQImage: (" << width << ", " << height << ")\n";

	QImage img = QImage(width, height, 32, 0, QImage::LittleEndian);
	
	if (srcProfile == 0 || dstProfile == 0) {
		kdDebug() << "Going to transform without profiles\n";

		// XXX: Temporary copy from cmyka.cc
		Q_INT32 i = 0;
		uchar *j = img.bits();
		
		while ( i < width * height * depth() ) {
			QUANTUM k = *( data + i + PIXEL_BLACK );
			QUANTUM c = *( data + i + PIXEL_CYAN );
			QUANTUM m = *( data + i + PIXEL_MAGENTA );
			QUANTUM y = *( data + i + PIXEL_YELLOW );
			
			c = c * ( QUANTUM_MAX - k) + k;
			m = m * ( QUANTUM_MAX - k) + k;
			y = y * ( QUANTUM_MAX - k) + k;
			
			// XXX: Temporary copy
			const PIXELTYPE PIXEL_BLUE = 0;
			const PIXELTYPE PIXEL_GREEN = 1;
			const PIXELTYPE PIXEL_RED = 2;
			const PIXELTYPE PIXEL_ALPHA = 3;

			*( j + PIXEL_ALPHA ) = OPACITY_OPAQUE ;
			*( j + PIXEL_RED )   = QUANTUM_MAX - c;
			*( j + PIXEL_GREEN ) = QUANTUM_MAX - m;
			*( j + PIXEL_BLUE )  = QUANTUM_MAX - y;
			
			i += 4;
			j += 4; // Because we're hard-coded 32 bits deep, 4 bytes
                
		}

	}
	else {
		kdDebug() << "Going to transform with profiles\n";

		// Do a nice calibrated conversion
		KisStrategyColorSpaceSP dstCS = KisColorSpaceRegistry::instance() -> get("RGBA");
		convertPixelsTo(const_cast<QUANTUM *>(data), srcProfile, 
				img.bits(), dstCS, dstProfile,
				width * height, renderingIntent);
	}

	return img;
}


void KisStrategyColorSpaceCMYK::bitBlt(Q_INT32 stride,
				       QUANTUM *dst,
				       Q_INT32 dststride,
				       QUANTUM *src,
				       Q_INT32 srcstride,
				       QUANTUM opacity,
				       Q_INT32 rows,
				       Q_INT32 cols,
				       CompositeOp op)
{
	Q_INT32 linesize = stride * sizeof(QUANTUM) * cols;
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
		if (opacity == OPACITY_TRANSPARENT) return;

		while (rows-- > 0) {
			d = dst;
			s = src;
			for (i = cols; i > 0; i--, d += stride, s += stride) {
				d[PIXEL_CYAN] = (d[PIXEL_CYAN] * OPACITY_OPAQUE  + s[PIXEL_CYAN] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
				d[PIXEL_MAGENTA] = (d[PIXEL_MAGENTA] * OPACITY_OPAQUE + s[PIXEL_MAGENTA] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
				d[PIXEL_YELLOW] = (d[PIXEL_YELLOW] * OPACITY_OPAQUE + s[PIXEL_YELLOW] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
				d[PIXEL_BLACK] = (d[PIXEL_BLACK] * OPACITY_OPAQUE + s[PIXEL_BLACK] * opacity + QUANTUM_MAX / 2) / QUANTUM_MAX;
			}
			dst += dststride;
			src += srcstride;
		}
	}

}

