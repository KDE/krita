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

#include "kis_image.h"
#include "kis_strategy_colorspace_cmyk.h"
#include "tiles/kispixeldata.h"
#include "kis_iterators_pixel.h"

#include "kis_resource.h"
#include "kis_resourceserver.h"
#include "kis_resource_mediator.h"
#include "kis_profile.h"

namespace {
	const Q_INT32 MAX_CHANNEL_CMYK = 4;
	// Is it actually possible to have transparency with CMYK?
	const Q_INT32 MAX_CHANNEL_CMYKA = 5;
}




KisStrategyColorSpaceCMYK::KisStrategyColorSpaceCMYK(bool alpha) :
	KisStrategyColorSpace()
{
// 	setProfile(cmsCreateNullProfile());
	m_alpha = alpha;

	m_channels.push_back(new KisChannelInfo(i18n("cyan"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("magenta"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("yellow"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("black"), 3, COLOR));


	if (alpha) {
		m_name = "CMYK/Alpha";
		// Custom definition for cmyka
		m_cmType = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | EXTRA_SH(1));
		m_channels.push_back(new KisChannelInfo(i18n("alpha"), 4, ALPHA));
	} else {
		m_name = "CMYK";
		m_cmType = TYPE_CMYK_8;
	}
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
#if 0
	kdDebug() << "KisStrategyColorSpaceCMYK::nativeColor: "
		  << "R: " << c.R()
		  << ", G: " << c.G()
		  << ", B: " << c.B()
		  << " -- " 
		  << " C: " << c.C()
		  << ", M: " << c.M()
		  << ", Y: " << c.Y()
		  << ", K: " << c.K()
		  << "\n";
#endif
	dst[PIXEL_CYAN] = upscale( c.C() );
	dst[PIXEL_MAGENTA] = upscale( c.M() );
	dst[PIXEL_YELLOW] = upscale( c.Y() );
	dst[PIXEL_BLACK] = upscale( c.K() );

	if (m_alpha)
		dst[PIXEL_CMYK_ALPHA] = opacity;
       
}

void KisStrategyColorSpaceCMYK::toKoColor(const QUANTUM *src, KoColor *c)
{
	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
}

void KisStrategyColorSpaceCMYK::toKoColor(const QUANTUM *src, KoColor *c, QUANTUM *opacity)
{
	c -> setCMYK(downscale(src[PIXEL_CYAN]), downscale(src[PIXEL_MAGENTA]), downscale(src[PIXEL_YELLOW]), downscale(src[PIXEL_BLACK]));
	if (m_alpha)
		*opacity = src[PIXEL_CMYK_ALPHA];
	else
		*opacity = OPACITY_OPAQUE;
}

vKisChannelInfoSP KisStrategyColorSpaceCMYK::channels() const
{
	return m_channels;
}

bool KisStrategyColorSpaceCMYK::alpha() const
{
	return m_alpha;
}

Q_INT32 KisStrategyColorSpaceCMYK::depth() const
{
	if (m_alpha)
		return MAX_CHANNEL_CMYKA;
	else
		return MAX_CHANNEL_CMYK;
}

Q_INT32 KisStrategyColorSpaceCMYK::nColorChannels() const
{
	return MAX_CHANNEL_CMYK;
}

QImage KisStrategyColorSpaceCMYK::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height, Q_INT32 stride) const 
{
	QImage img(width, height, 32, 0, QImage::LittleEndian);
	Q_INT32 i = 0;
	uchar *j = img.bits();

	// XXX: Convert using littlecms

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
				if (m_alpha) {
					d[PIXEL_CMYK_ALPHA] = alpha; // XXX: this is certainly incorrect.
				}
			}
			dst += dststride;
			src += srcstride;
		}
	}

}

