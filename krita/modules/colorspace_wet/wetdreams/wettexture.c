/* synthesize a surface texture */

#include <stdlib.h>
#include <math.h>
#include "wetpix.h"

void
wet_layer_maketexture(WetLayer * layer,
		      double height, double blurh, double blurv)
{
	int x, y;
	int width = layer->width;
	int lheight = layer->height;
	int rowstride = layer->rowstride;
	WetPix *wet_line = layer->buf;
	double hscale = 128 * height / RAND_MAX;
	int lh;
	int ibh, ibv;

	ibh = floor(256 * blurh + 0.5);
#ifdef VERBOSE
	g_print("ibh = %d\n", ibh);
#endif
	ibv = floor(256 * blurv + 0.5);

	for (y = 0; y < lheight; y++) {
		for (x = 0; x < width; x++) {
			wet_line[x].h = floor(128 + hscale * rand());
		}
		/*      g_print ("%d\n", wet_line[0].h); */
		wet_line += rowstride;
	}

	wet_line = layer->buf;
	for (y = 0; y < lheight; y++) {
		lh = wet_line[0].h;
		for (x = 1; x < width; x++) {
			wet_line[x].h +=
			    ((lh - wet_line[x].h) * ibh + 128) >> 8;
			lh = wet_line[x].h;
		}
		wet_line += rowstride;
	}

#if 0
	for (x = 0; x < width; x++) {
		wet_line = layer->buf + x;
		lh = wet_line[0].h;
		for (y = 1; y < lheight; y++) {
			wet_line += rowstride;
			wet_line[0].h +=
			    ((lh - wet_line[0].h) * ibv + 128) >> 8;
			lh = wet_line[0].h;
		}
	}
#endif
}

void wet_layer_clone_texture(WetLayer * dst, WetLayer * src)
{
	int x, y;
	int width = src->width;
	WetPix *dst_line = dst->buf;
	WetPix *src_line = src->buf;

	for (y = 0; y < src->height; y++) {
		for (x = 0; x < width; x++) {
			dst_line[x].h = src_line[x].h;
		}
		dst_line += dst->rowstride;
		src_line += src->rowstride;
	}
}

void
wet_pack_maketexture(WetPack * pack,
		     double height, double blurh, double blurv)
{
	int i;

	wet_layer_maketexture(pack->layers[0], height, blurh, blurv);
	for (i = 1; i < pack->n_layers; i++)
		wet_layer_clone_texture(pack->layers[i], pack->layers[0]);
}
