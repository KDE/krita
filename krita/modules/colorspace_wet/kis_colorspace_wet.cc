/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger
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

#include <config.h>
#include LCMS_HEADER

#include <qimage.h>

#include <klocale.h>
#include <kdebug.h>

#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_image.h"
#include "kis_colorspace_wet.h"
#include "kis_iterators_pixel.h"
namespace {

	void wetPixToDouble(WetPixDbl * dst, WetPix *src)
	{
		dst->rd = (1.0 / 8192.0) * src->rd;
		dst->rw = (1.0 / 8192.0) * src->rw;
		dst->gd = (1.0 / 8192.0) * src->gd;
		dst->gw = (1.0 / 8192.0) * src->gw;
		dst->bd = (1.0 / 8192.0) * src->bd;
		dst->bw = (1.0 / 8192.0) * src->bw;
		dst->w = (1.0 / 8192.0) * src->w;
		dst->h = (1.0 / 8192.0) * src->h;
	}

	void wetPixFromDouble(WetPix * dst, WetPixDbl *src)
	{
		int v;

		v = floor (8192.0 * src->rd + 0.5);
		if (v < 0) v = 0;
		if (v > 65535) v = 65535;
		dst->rd = v;
		
		v = floor (8192.0 * src->rw + 0.5);
		if (v < 0) v = 0;
		if (v > 65535) v = 65535;
		dst->rw = v;
		
		v = floor (8192.0 * src->gd + 0.5);
		if (v < 0) v = 0;
		if (v > 65535) v = 65535;
		dst->gd = v;
		
		v = floor (8192.0 * src->gw + 0.5);
		if (v < 0) v = 0;
		if (v > 65535) v = 65535;
		dst->gw = v;
		
		v = floor (8192.0 * src->bd + 0.5);
		if (v < 0) v = 0;
		if (v > 65535) v = 65535;
		dst->bd = v;
		
		v = floor (8192.0 * src->bw + 0.5);
		if (v < 0) v = 0;
		if (v > 65535) v = 65535;
		dst->bw = v;
		
		v = floor (8192.0 * src->w + 0.5);
		if (v < 0) v = 0;
		if (v > 511) v = 511;
		dst->w = v;
		
		v = floor (8192.0 * src->h + 0.5);
		if (v < 0) v = 0;
		if (v > 511) v = 511;
		dst->h = v;

	}


}

KisColorSpaceWet::KisColorSpaceWet() :
	KisStrategyColorSpace(KisID("WET", i18n("Watercolors")), 0, icMaxEnumData)
{
	wet_init_render_tab();
}


KisColorSpaceWet::~KisColorSpaceWet()
{
}

void KisColorSpaceWet::nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP /*profile*/)
{
}

void KisColorSpaceWet::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst, KisProfileSP /*profile*/)
{
}

void KisColorSpaceWet::toQColor(const QUANTUM *src, QColor *c, KisProfileSP /*profile*/)
{
	Q_UINT8 * rgb = new Q_UINT8[3];
	wet_composite(rgb, (WetPix*)src);
	c -> setRgb(rgb[0], rgb[1], rgb[2]);
	delete[]rgb;
}

void KisColorSpaceWet::toQColor(const QUANTUM *src, QColor *c, QUANTUM *opacity, KisProfileSP /*profile*/)
{
	toQColor(src, c);
}

vKisChannelInfoSP KisColorSpaceWet::channels() const
{
	return m_channels;
}

bool KisColorSpaceWet::alpha() const
{
	return false;
}

Q_INT32 KisColorSpaceWet::nChannels() const
{
	return 8;
}

Q_INT32 KisColorSpaceWet::nColorChannels() const
{
	return 6;
}

Q_INT32 KisColorSpaceWet::nSubstanceChannels() const
{
        return 2;
}


Q_INT32 KisColorSpaceWet::pixelSize() const
{
	return 16; // This color strategy wants an unsigned short for each channel.
}


QImage KisColorSpaceWet::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);
	return img;
}

void KisColorSpaceWet::bitBlt(Q_INT32 stride,
			    QUANTUM *dst,
			    Q_INT32 dststride,
			    const QUANTUM *src,
			    Q_INT32 srcstride,
			    QUANTUM opacity,
			    Q_INT32 rows,
			    Q_INT32 cols,
			    CompositeOp op)
{
}

void KisColorSpaceWet::wet_init_render_tab()
{
	int i;

	double d;
	int a, b;

	wet_render_tab = new Q_UINT32[4096];

	for (i = 0; i < 4096; i++)
	{
		d = i * (1.0 / 512.0);

		if (i == 0)
			a = 0;
		else
			a = floor (0xff00 / i + 0.5);

		b = floor (0x8000 * exp (-d) + 0.5);
		wet_render_tab[i] = (a << 16) | b;
	}

}

void KisColorSpaceWet::wet_composite(Q_UINT8 *rgb, WetPix * wet)
{
	int r, g, b;
	int d, w;
	int ab;
	int wa;
	
	r = rgb[0];
	w = wet[0].rw >> 4;
	d = wet[0].rd >> 4;
	
	ab = wet_render_tab[d];
	wa = (w * (ab >> 16) + 0x80) >> 8;
	r = wa + (((r - wa) * (ab & 0xffff) + 0x4000) >> 15);
	rgb[0] = r;
	
	
	g = rgb[1];
	w = wet[0].gw >> 4;
	d = wet[0].gd >> 4;
	d = d >= 4096 ? 4095 : d;
	ab = wet_render_tab[d];
	wa = (w * (ab >> 16) + 0x80) >> 8;
	g = wa + (((g - wa) * (ab & 0xffff) + 0x4000) >> 15);
	rgb[1] = g;
	
	b = rgb[2];
	w = wet[0].bw >> 4;
	d = wet[0].bd >> 4;
	d = d >= 4096 ? 4095 : d;
	ab = wet_render_tab[d];
	wa = (w * (ab >> 16) + 0x80) >> 8;
	b = wa + (((b - wa) * (ab & 0xffff) + 0x4000) >> 15);
	rgb[2] = b;

}
