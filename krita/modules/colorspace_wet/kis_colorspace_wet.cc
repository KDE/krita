/*
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

	m_channels.push_back(new KisChannelInfo(i18n("red concentration"), 0, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("myth red"), 1, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("green concentration"), 2, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("myth green"), 3, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("blue concentration"), 4, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("myth blue"), 5, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("water volume"), 6, SUBSTANCE));
	m_channels.push_back(new KisChannelInfo(i18n("paper height"), 7, SUBSTANCE));

	m_channels.push_back(new KisChannelInfo(i18n("adsorbed red concentration"), 8, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed myth red"), 9, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed green concentration"), 10, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed myth green"), 11, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed blue concentration"), 12, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed myth blue"), 13, COLOR));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed water volume"), 14, SUBSTANCE));
	m_channels.push_back(new KisChannelInfo(i18n("adsorbed paper height"), 15, SUBSTANCE));

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
	// XXX: Define a class that combines QColor, wet paint color and name.
	if (r == 240 && g == 32 && b == 160) {
		// Quinacridone Rose
		memcpy(dst, &m_paintbox[0], sizeof(WetPix));
	} else if (r == 159 && g == 88 && b == 43) {
		// Indian Red
		memcpy(dst, &m_paintbox[1], sizeof(WetPix));
	} else if (r == 254 && g == 220  && b == 64) {
		// Cadmium Yellow
		memcpy(dst, &m_paintbox[2], sizeof(WetPix));
	} else if (r == 36 && g == 180 && b == 32) {
		// Hookers Green
		memcpy(dst, &m_paintbox[3], sizeof(WetPix));
	} else if (r == 16 && g == 185 && b == 215) {
		// Cerulean Blue
		memcpy(dst, &m_paintbox[4], sizeof(WetPix));
	} else if (r == 96 && g == 32 && b == 8) {
		// Burnt Umber
		memcpy(dst, &m_paintbox[5], sizeof(WetPix));
	} else if (r ==  254 && g == 96 && b == 8) {
		// Cadmium Red
		memcpy(dst, &m_paintbox[6], sizeof(WetPix));
	} else if (r == 255 && g == 136 && b == 8) {
		// Brilliant Orange
		memcpy(dst, &m_paintbox[7], sizeof(WetPix));
	} else if (r == 240 && g == 199 && b == 8) {
		// Hansa Yellow
		memcpy(dst, &m_paintbox[8], sizeof(WetPix));
	} else if (r == 96 && g == 170 && b == 130) {
		// Phthalo Green
		memcpy(dst, &m_paintbox[9], sizeof(WetPix));
	} else if (r == 48 && g == 32 && b == 170) {
		// French Ultramarine
		memcpy(dst, &m_paintbox[10], sizeof(WetPix));
	} else if (r == 118 && g == 16 && b == 135) {
		// Interference Lilac
		memcpy(dst, &m_paintbox[11], sizeof(WetPix));
	} else if (r == 254 && g == 254 && b == 254) {
		// Titanium White
		memcpy(dst, &m_paintbox[12], sizeof(WetPix));
	} else if (r == 64 && g == 64 && b == 74) {
		// Ivory Black
		memcpy(dst, &m_paintbox[13], sizeof(WetPix));
	} else {
		// Pure water
		memcpy(dst, &m_paintbox[14], sizeof(WetPix));
	}
	// XXX: Maybe somehow do something useful with QColor that don't correspond to paint from the paintbox.
}

void KisColorSpaceWet::nativeColor(const QColor& c, QUANTUM  /*opacity*/, QUANTUM *dst, KisProfileSP /*profile*/)
{
	nativeColor(c, dst);
}

void KisColorSpaceWet::toQColor(const QUANTUM *src, QColor *c, KisProfileSP /*profile*/)
{
	Q_UINT8 * rgb = new Q_UINT8[3];
	memset(rgb, 255, 3);

	// Composite the two layers in each pixelSize

	// First the adsorption layers
	wet_composite(rgb, (WetPix*)(src + 16));

	// Then the paint layer (which comes first in our double-packed pixel)
	wet_composite(rgb,  (WetPix*)(src));

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
	return 16;
}

Q_INT32 KisColorSpaceWet::nColorChannels() const
{
	return 12;
}

Q_INT32 KisColorSpaceWet::nSubstanceChannels() const
{
        return 4;
}


Q_INT32 KisColorSpaceWet::pixelSize() const
{
	return 32; // This color strategy wants an unsigned short for each
		   // channel, and every pixel consists of two wetpix structs
		   // -- even though for many purposes we need only one wetpix
		   // struct.
}


// XXX: use profiles to display correctly on calibrated displays.
QImage KisColorSpaceWet::convertToQImage(const QUANTUM *data, Q_INT32 width, Q_INT32 height,
				         KisProfileSP /*srcProfile*/, KisProfileSP /*dstProfile*/,
				         Q_INT32 /*renderingIntent*/)
{

	QImage img(width, height, 32, 0, QImage::LittleEndian);

        Q_UINT8 *rgb = (Q_UINT8*) img.bits();

	// Clear to white -- the following code actually composits the contents of the
	// wet pixels with the contents of the image buffer, so they need to be
	// prepared
	memset(rgb, 255, width * height * sizeof(Q_UINT32));

	// Composite the two layers in each pixelSize

	Q_INT32 i = 0;
	while ( i < width * height * 32) {

		// First the adsorption layers
		wet_composite(rgb, (WetPix*)data + i + 16);

		// Then the paint layer (which comes first in our double-packed pixel)
		wet_composite(rgb,  (WetPix*)data + i);

		i += 32;
		rgb += sizeof(Q_UINT32); // Because the QImage is 4 bytes deep.

	}

	// Display the wet stripes -- this only works if we have at least three scanlines in height,
	// because otherwise the phase trick won't work.

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
			      QUANTUM /*opacity*/,
			      Q_INT32 rows,
			      Q_INT32 cols,
			      CompositeOp /*op*/)
{
	if (rows <= 0 || cols <= 0)
		return;

	QUANTUM *d;
	const QUANTUM *s;

	Q_INT32 linesize = stride * sizeof(QUANTUM) * cols;

	// Just copy the src onto the dst, we don't do fancy things here,
	// we do those in the paint op, because we need pressure to determine
	// paint deposition.

	d = dst;
	s = src;
	while (rows-- > 0) {
		memcpy(d, s, linesize);
		d += dststride;
		s += srcstride;
	}

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

void wet_render_wetness(Q_UINT8 * rgb, Q_INT32 rgb_rowstride, WetPix * pix, Q_INT32 height)
{
//         static int wet_phase = 0;
//         int x, y;
//         byte *rgb_line = rgb;
//         WetPix *wet_line = layer->buf + (y0 * layer->rowstride) + x0;
//         int highlight;
//
//         for (y = 0; y < height; y++) {
//                 byte *rgb_ptr = rgb_line;
//                 WetPix *wet_ptr = wet_line;
//                 for (x = 0; x < width; x++) {
//                         if (((x + y) & 3) == wet_phase) {
//                                 highlight = 255 - (wet_ptr[0].w >> 1);
//                                 if (highlight < 255) {
//                                         rgb_ptr[0] =
//                                             255 -
//                                             (((255 -
//                                                rgb_ptr[0]) *
//                                               highlight) >> 8);
//                                         rgb_ptr[1] =
//                                             255 -
//                                             (((255 -
//                                                rgb_ptr[1]) *
//                                               highlight) >> 8);
//                                         rgb_ptr[2] =
//                                             255 -
//                                             (((255 -
//                                                rgb_ptr[2]) *
//                                               highlight) >> 8);
//                                 }
//                         }
//                         rgb_ptr += 3;
//                         wet_ptr++;
//                 }
//                 rgb_line += rgb_rowstride;
//                 wet_line += layer->rowstride;
//         }
//         wet_phase += 1;
//         wet_phase &= 3;
}
