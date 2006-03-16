/* Routines for manipulating wet pixels.

   Copyright 1999 Raph Levien <raph@gimp.org>

   Released under GPL.

   A wet pixel is a sequence of eight bytes, arranged as follows:

   Red value when composited over black
   Green value when composited over black
   Blue value when composited over black
   Volume of water
   Red value when composited over white
   Green value when composited over white
   Blue value when composited over white
   Height of paper surface

*/

#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include "wetpix.h"

u32 *wet_render_tab = NULL;

static void wet_init_render_tab(void)
{
	int i;
	double d;
	int a, b;

	wet_render_tab = g_new(u32, 4096);
	for (i = 0; i < 4096; i++) {
		d = i * (1.0 / 512.0);
		if (i == 0)
			a = 0;
		else
			a = floor(0xff00 / i + 0.5);
		b = floor(0x8000 * exp(-d) + 0.5);
#if 0
		g_print("%d: %x %x\n", i, a, b);
#endif
		wet_render_tab[i] = (a << 16) | b;
	}
}

void
wet_composite(byte * rgb, int rgb_rowstride,
	      WetPix * wet, int wet_rowstride,
	      int width, int height)
{
	int x, y;
	byte *rgb_line = rgb;
	WetPix *wet_line = wet;

	if (wet_render_tab == NULL)
		wet_init_render_tab();

	for (y = 0; y < height; y++) {
		byte *rgb_ptr = rgb_line;
		WetPix *wet_ptr = wet_line;
		for (x = 0; x < width; x++) {
			int r, g, b;
			int d, w;
			int ab;
			int wa;

			r = rgb_ptr[0];
			w = wet_ptr[0].rw >> 4;
			d = wet_ptr[0].rd >> 4;
			/*
			   d = d >= 4096 ? 4095 : d;
			 */
			ab = wet_render_tab[d];
			wa = (w * (ab >> 16) + 0x80) >> 8;
			r = wa +
			    (((r - wa) * (ab & 0xffff) + 0x4000) >> 15);
			rgb_ptr[0] = r;

#if 0
			if (x == 128 && y == 128) {
				g_print("w %d d %d r %d\n", w, d, r);
			}
#endif

			g = rgb_ptr[1];
			w = wet_ptr[0].gw >> 4;
			d = wet_ptr[0].gd >> 4;
			d = d >= 4096 ? 4095 : d;
			ab = wet_render_tab[d];
			wa = (w * (ab >> 16) + 0x80) >> 8;
			g = wa +
			    (((g - wa) * (ab & 0xffff) + 0x4000) >> 15);
			rgb_ptr[1] = g;

			b = rgb_ptr[2];
			w = wet_ptr[0].bw >> 4;
			d = wet_ptr[0].bd >> 4;
			d = d >= 4096 ? 4095 : d;
			ab = wet_render_tab[d];
			wa = (w * (ab >> 16) + 0x80) >> 8;
			b = wa +
			    (((b - wa) * (ab & 0xffff) + 0x4000) >> 15);
			rgb_ptr[2] = b;

			rgb_ptr += 3;
			wet_ptr++;
		}
		rgb_line += rgb_rowstride;
		wet_line += wet_rowstride;
	}
}

void
wet_render_wetness(byte * rgb, int rgb_rowstride,
		   WetLayer * layer, int x0, int y0, int width, int height)
{
	static int wet_phase = 0;
	int x, y;
	byte *rgb_line = rgb;
	WetPix *wet_line = layer->buf + (y0 * layer->rowstride) + x0;
	int highlight;

	for (y = 0; y < height; y++) {
		byte *rgb_ptr = rgb_line;
		WetPix *wet_ptr = wet_line;
		for (x = 0; x < width; x++) {
			if (((x + y) & 3) == wet_phase) {
				highlight = 255 - (wet_ptr[0].w >> 1);
				if (highlight < 255) {
					rgb_ptr[0] =
					    255 -
					    (((255 -
					       rgb_ptr[0]) *
					      highlight) >> 8);
					rgb_ptr[1] =
					    255 -
					    (((255 -
					       rgb_ptr[1]) *
					      highlight) >> 8);
					rgb_ptr[2] =
					    255 -
					    (((255 -
					       rgb_ptr[2]) *
					      highlight) >> 8);
				}
			}
			rgb_ptr += 3;
			wet_ptr++;
		}
		rgb_line += rgb_rowstride;
		wet_line += layer->rowstride;
	}
	wet_phase += 1;
	wet_phase &= 3;
}

void
wet_composite_layer(byte * rgb, int rgb_rowstride,
		    WetLayer * layer,
		    int x0, int y0, int width, int height)
{
	/* todo: sanitycheck bounds */
	wet_composite(rgb, rgb_rowstride,
		      layer->buf + (y0 * layer->rowstride) + x0,
		      layer->rowstride, width, height);
}

void
wet_pack_render(byte * rgb, int rgb_rowstride,
		WetPack * pack, int x0, int y0, int width, int height)
{
	int y;
	byte *rgb_line = rgb;
	int i;

	/* clear rgb buffer to white */
	for (y = 0; y < height; y++) {
		memset(rgb_line, 255, width * 3);
		rgb_line += rgb_rowstride;
	}

	/* black stripe */
/*  rgb_line = rgb;
  for (y = y0; y < 8 && y < y0 + height; y++)
    {
      memset (rgb_line, 0, width * 3);
      rgb_line += rgb_rowstride;
    }
*/

	for (i = 0; i < pack->n_layers; i++)
		wet_composite_layer(rgb, rgb_rowstride,
				    pack->layers[i],
				    x0, y0, width, height);

	wet_render_wetness(rgb, rgb_rowstride,
			   pack->layers[pack->n_layers - 1],
			   x0, y0, width, height);
}

WetLayer *wet_layer_new(int width, int height)
{
	WetLayer *layer;

	layer = g_new(WetLayer, 1);

	layer->buf = g_new(WetPix, width * height);
	layer->width = width;
	layer->height = height;
	layer->rowstride = width;

	return layer;
}

void wet_layer_clear(WetLayer * layer)
{
	int x, y;
	WetPix *wet_line = layer->buf;
	int width = layer->width;

	for (y = 0; y < layer->height; y++) {
		for (x = 0; x < width; x++) {
			/* transparent, dry, smooth */
			wet_line[x].rd = 0;
			wet_line[x].rw = 0;
			wet_line[x].gd = 0;
			wet_line[x].gw = 0;
			wet_line[x].bd = 0;
			wet_line[x].bw = 0;
			wet_line[x].w = 0;
			wet_line[x].h = 128;
		}
		wet_line += layer->rowstride;
	}
}

WetPack *wet_pack_new(int width, int height)
{
	WetPack *pack;

	pack = g_new(WetPack, 1);

	pack->n_layers = 2;
	pack->layers = g_new(WetLayer *, pack->n_layers);
	pack->layers[0] = wet_layer_new(width, height);
	wet_layer_clear(pack->layers[0]);
	pack->layers[1] = wet_layer_new(width, height);
	wet_layer_clear(pack->layers[1]);

	return pack;
}

void wet_pix_to_double(WetPixDbl * dst, WetPix * src)
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

void wet_pix_from_double(WetPix * dst, WetPixDbl * src)
{
	int v;

	v = floor(8192.0 * src->rd + 0.5);
	if (v < 0)
		v = 0;
	if (v > 65535)
		v = 65535;
	dst->rd = v;

	g_print("src->rd = %f, dst->rd = %d\n", src->rd, dst->rd);

	v = floor(8192.0 * src->rw + 0.5);
	if (v < 0)
		v = 0;
	if (v > 65535)
		v = 65535;
	dst->rw = v;

	v = floor(8192.0 * src->gd + 0.5);
	if (v < 0)
		v = 0;
	if (v > 65535)
		v = 65535;
	dst->gd = v;

	v = floor(8192.0 * src->gw + 0.5);
	if (v < 0)
		v = 0;
	if (v > 65535)
		v = 65535;
	dst->gw = v;

	v = floor(8192.0 * src->bd + 0.5);
	if (v < 0)
		v = 0;
	if (v > 65535)
		v = 65535;
	dst->bd = v;

	v = floor(8192.0 * src->bw + 0.5);
	if (v < 0)
		v = 0;
	if (v > 65535)
		v = 65535;
	dst->bw = v;

	v = floor(8192.0 * src->w + 0.5);
	if (v < 0)
		v = 0;
	if (v > 511)
		v = 511;
	dst->w = v;
#if 0
	g_print("src->w = %f, dst->w = %d\n", src->w, dst->w);
#endif

	v = floor(8192.0 * src->h + 0.5);
	if (v < 0)
		v = 0;
	if (v > 511)
		v = 511;
	dst->h = v;

}
