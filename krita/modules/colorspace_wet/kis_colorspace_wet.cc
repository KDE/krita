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
	static const WetPix m_paint = { 707, 0, 707, 0, 707, 0, 240, 0 };

	/* colors from Curtis et al, Siggraph 97 */

	static const WetPix m_paintbox[] = {
		{496, 0, 16992, 0, 3808, 0, 0, 0},
		{16992, 9744, 21712, 6400, 25024, 3296, 0, 0},
		{6512, 6512, 6512, 4880, 11312, 0, 0, 0},
		{16002, 0, 2848, 0, 16992, 0, 0, 0},
		{22672, 0, 5328, 2272, 4288, 2640, 0, 0},
		{8000, 0, 16992, 0, 28352, 0, 0, 0},
		{5696, 5696, 12416, 2496, 28352, 0, 0, 0},
		{0, 0, 5136, 0, 28352, 0, 0, 0},
		{2320, 1760, 7344, 4656, 28352, 0, 0, 0},
		{8000, 0, 3312, 0, 5504, 0, 0, 0},
		{13680, 0, 16992, 0, 3312, 0, 0, 0},
		{5264, 5136, 1056, 544, 6448, 6304, 0, 0},
		{11440, 11440, 11440, 11440, 11440, 11440, 0, 0},
		{11312, 0, 11312, 0, 11312, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0} };

	static const int m_nPaints = 15;
}

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

KisColorSpaceWet::KisColorSpaceWet() :
	KisStrategyColorSpace(KisID("WET", i18n("Watercolors")), 0, icMaxEnumData)
{
	wet_init_render_tab();
	m_paintNames << i18n("Quinacridone Rose")
		<< i18n("Indian Red")
		<< i18n("Cadmium Yellow")
		<< i18n("Hookers Green")
		<< i18n("Cerulean Blue")
		<< i18n("Burnt Umber")
		<< i18n("Cadmium Red")
		<< i18n("Brilliant Orange")
		<< i18n("Hansa Yellow")
		<< i18n("Phthalo Green")
		<< i18n("French Ultramarine")
		<< i18n("Interference Lilac")
		<< i18n("Titanium White")
		<< i18n("Ivory Black")
		<< i18n("Pure Water");
}


KisColorSpaceWet::~KisColorSpaceWet()
{
}

void KisColorSpaceWet::nativeColor(const QColor& c, QUANTUM *dst, KisProfileSP /*profile*/)
{
	Q_INT32 r, g, b;
	c.getRgb(&r, &g, &b);

	WetPix * p = (WetPix*)dst;

	// Translate the special QCOlors from our paintbox to wetpaint paints.
	if (r == 240 && g == 32 && b == 160) {
		// Quinacridone Rose
		memcpy(dst, &m_paintbox[0], sizeof(WetPix));
	} else if (r == 159 && g == 88 && b == 43) {
		// Indian Red
		memcpy(dst, &m_paintbox[1], sizeof(WetPix));
	} else if (r == 254, 220, 64) {
		// Cadmium Yellow
		memcpy(dst, &m_paintbox[2], sizeof(WetPix));
	} else if (r == 36, 180, 32) {
		// Hookers Green
		memcpy(dst, &m_paintbox[3], sizeof(WetPix));
	} else if (r == 16, 185, 215) {
		// Cerulean Blue
		memcpy(dst, &m_paintbox[4], sizeof(WetPix));
	} else if (r == 96, 32, 8) {
		// Burnt Umber
		memcpy(dst, &m_paintbox[5], sizeof(WetPix));
	} else if (r ==  254, 96, 8) {
		// Cadmium Red
		memcpy(dst, &m_paintbox[6], sizeof(WetPix));
	} else if (r == 255, 136, 8) {
		// Brilliant Orange
		memcpy(dst, &m_paintbox[7], sizeof(WetPix));
	} else if (r == 240, 199, 8) {
		// Hansa Yellow
		memcpy(dst, &m_paintbox[8], sizeof(WetPix));
	} else if (r == 96, 170, 130) {
		// Phthalo Green
		memcpy(dst, &m_paintbox[9], sizeof(WetPix));
	} else if (r == 48, 32, 170) {
		// French Ultramarine
		memcpy(dst, &m_paintbox[10], sizeof(WetPix));
	} else if (r == 118, 16, 135) {
		// Interference Lilac
		memcpy(dst, &m_paintbox[11], sizeof(WetPix));
	} else if (r == 254, 254, 254) {
		// Titanium White
		memcpy(dst, &m_paintbox[12], sizeof(WetPix));
	} else if (r == 64, 64, 74) {
		// Ivory Black
		memcpy(dst, &m_paintbox[13], sizeof(WetPix));
	} else {
		// Pure water
		memcpy(dst, &m_paintbox[14], sizeof(WetPix));
	}
}

void KisColorSpaceWet::nativeColor(const QColor& c, QUANTUM  /*opacity*/, QUANTUM *dst, KisProfileSP /*profile*/)
{
	nativeColor(c, dst);
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
	return 32; // This color strategy wants an unsigned short for each
		   // channel, and every pixel consists of two wetpix structs
		   // -- even though for many purposes we need only one wetpix
		   // struct.
}


QImage KisColorSpaceWet::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

        int y, i;

        uchar *rgb = img.bits();

//         for (i = 0; i < pack->n_layers; i++)
//                 wet_composite_layer(rgb, rgb_rowstride,
//                                     pack->layers[i],
//                                     x0, y0, width, height);
//
//         wet_render_wetness(rgb, rgb_rowstride,
//                            pack->layers[pack->n_layers - 1],
//                            x0, y0, width, height);
//

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
