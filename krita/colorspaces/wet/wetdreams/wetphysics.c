/* Cool physics functions for wet paint */

#include <gtk/gtk.h>
#include <math.h>
#include "wetpix.h"
#include "wetphysics.h"

/* symmetric combine, i.e. wet mixing.

   This does not set the dst h field.
*/
void wet_pix_combine(WetPixDbl * dst, WetPixDbl * src1, WetPixDbl * src2)
{
	dst->rd = src1->rd + src2->rd;
	dst->rw = src1->rw + src2->rw;
#if 0
	g_print("rd %f rw %f\n", dst->rd, dst->rw);
#endif
	dst->gd = src1->gd + src2->gd;
	dst->gw = src1->gw + src2->gw;
	dst->bd = src1->bd + src2->bd;
	dst->bw = src1->bw + src2->bw;
	dst->w = src1->w + src2->w;
#if 0
	g_print("%f + %f -> %f\n", src1->w, src2->w, dst->w);
#endif
}

void wet_pix_dilute(WetPixDbl * dst, WetPix * src, double dilution)
{
	double scale = dilution * (1.0 / 8192.0);


	dst->rd = src->rd * scale;
#if 0
	g_print("dilution %f scale %f rd %f\n", dilution, scale, dst->rd);
#endif
	dst->rw = src->rw * scale;
	dst->gd = src->gd * scale;
	dst->gw = src->gw * scale;
	dst->bd = src->bd * scale;
	dst->bw = src->bw * scale;
	dst->w = src->w * (1.0 / 8192.0);
	dst->h = src->h * (1.0 / 8192.0);
}

void wet_pix_reduce(WetPixDbl * dst, WetPix * src, double dilution)
{
	wet_pix_dilute(dst, src, dilution);
	dst->w *= dilution;
}

/* allows visualization of adsorption by rotating the hue 120 degrees */
/* layer-merge combining. src1 is the top layer

   This does not set the dst h or w fields.
*/
void
wet_pix_merge(WetPixDbl * dst, WetPixDbl * src1, double dilution1,
	      WetPixDbl * src2)
{
	double d1, w1, d2, w2;
	double ed1, ed2;

	if (src1->rd < 1e-4) {
		dst->rd = src2->rd;
		dst->rw = src2->rw;
	} else if (src2->rd < 1e-4) {
		dst->rd = src1->rd * dilution1;
		dst->rw = src1->rw * dilution1;
	} else {
		d1 = src1->rd;
		w1 = src1->rw;
		d2 = src2->rd;
		w2 = src2->rw;
		dst->rd = d1 * dilution1 + d2;
		ed1 = exp(-d1 * dilution1);
		ed2 = exp(-d2);
		dst->rw = dst->rd * ((1 - ed1) * w1 / d1 +
				     ed1 * (1 - ed2) * w2 / d2) /
		    (1 - ed1 * ed2);
	}

	if (src1->gd < 1e-4) {
		dst->gd = src2->gd;
		dst->gw = src2->gw;
	} else if (src2->gd < 1e-4) {
		dst->gd = src1->gd * dilution1;
		dst->gw = src1->gw * dilution1;
	} else {
		d1 = src1->gd;
		w1 = src1->gw;
		d2 = src2->gd;
		w2 = src2->gw;
		dst->gd = d1 * dilution1 + d2;
		ed1 = exp(-d1 * dilution1);
		ed2 = exp(-d2);
		dst->gw = dst->gd * ((1 - ed1) * w1 / d1 +
				     ed1 * (1 - ed2) * w2 / d2) /
		    (1 - ed1 * ed2);
	}

	if (src1->bd < 1e-4) {
		dst->bd = src2->bd;
		dst->bw = src2->bw;
	} else if (src2->bd < 1e-4) {
		dst->bd = src1->bd * dilution1;
		dst->bw = src1->bw * dilution1;
	} else {
		d1 = src1->bd;
		w1 = src1->bw;
		d2 = src2->bd;
		w2 = src2->bw;
		dst->bd = d1 * dilution1 + d2;
		ed1 = exp(-d1 * dilution1);
		ed2 = exp(-d2);
		dst->bw = dst->bd * ((1 - ed1) * w1 / d1 +
				     ed1 * (1 - ed2) * w2 / d2) /
		    (1 - ed1 * ed2);
	}

}

void wet_flow(WetLayer * layer)
{
	/* XXX: Is this like a convolution operation? BSAR */
	int x, y;
	int width = layer->width;
	int height = layer->height;
	int rs = layer->rowstride;
	double *flow_t, *flow_b, *flow_l, *flow_r;
	double *fluid, *outflow;
	WetPix *wet_line = layer->buf;
	WetPix *wet_old;
	int my_height;
	int ix;
	double ft, fb, fl, fr; /* top, bottom, left, right */
	WetPixDbl wet_mix, wet_tmp;
	
	flow_t = g_new(double, width * height);
	flow_b = g_new(double, width * height);
	flow_l = g_new(double, width * height);
	flow_r = g_new(double, width * height);
	fluid = g_new(double, width * height);
	outflow = g_new(double, width * height);
	wet_old = g_new(WetPix, width * height);

	/* assumes rowstride == width */
	memcpy(wet_old, layer->buf, sizeof(WetPix) * width * height);

	ix = width + 1;
	for (y = 1; y < height - 1; y++) {
		wet_line += rs;
		for (x = 1; x < width - 1; x++) {
			if (wet_line[x].w > 0) {
				my_height = wet_line[x].h + wet_line[x].w;
				ft = (wet_line[x - rs].h +
				      wet_line[x - rs].w) - my_height;
				fb = (wet_line[x + rs].h +
				      wet_line[x + rs].w) - my_height;
				fl = (wet_line[x - 1].h +
				      wet_line[x - 1].w) - my_height;
				fr = (wet_line[x + 1].h +
				      wet_line[x + 1].w) - my_height;

				fluid[ix] =
				    0.4 * sqrt(wet_line[x].w * 1.0 /
					       255.0);

				/* smooth out the flow a bit */
				flow_t[ix] =
				    0.1 * (10 + ft * 0.75 - fb * 0.25);
				if (flow_t[ix] > 1)
					flow_t[ix] = 1;
				if (flow_t[ix] < 0)
					flow_t[ix] = 0;
				flow_b[ix] =
				    0.1 * (10 + fb * 0.75 - ft * 0.25);
				if (flow_b[ix] > 1)
					flow_b[ix] = 1;
				if (flow_b[ix] < 0)
					flow_b[ix] = 0;
				flow_l[ix] =
				    0.1 * (10 + fl * 0.75 - fr * 0.25);
				if (flow_l[ix] > 1)
					flow_l[ix] = 1;
				if (flow_l[ix] < 0)
					flow_l[ix] = 0;
				flow_r[ix] =
				    0.1 * (10 + fr * 0.75 - fl * 0.25);
				if (flow_r[ix] > 1)
					flow_r[ix] = 1;
				if (flow_r[ix] < 0)
					flow_r[ix] = 0;

				outflow[ix] = 0;
			}
			ix++;
		}
		ix += 2;
	}

	ix = width + 1;
	wet_line = layer->buf;
	for (y = 1; y < height - 1; y++) {
		wet_line += rs;
		for (x = 1; x < width - 1; x++) {
			if (wet_line[x].w > 0) {
				/* reduce flow in dry areas */
				flow_t[ix] *= fluid[ix] * fluid[ix - rs];
				outflow[ix - rs] += flow_t[ix];
				flow_b[ix] *= fluid[ix] * fluid[ix + rs];
				outflow[ix + rs] += flow_b[ix];
				flow_l[ix] *= fluid[ix] * fluid[ix - 1];
				outflow[ix - 1] += flow_l[ix];
				flow_r[ix] *= fluid[ix] * fluid[ix + 1];
				outflow[ix + 1] += flow_r[ix];
			}
			ix++;
		}
		ix += 2;
	}

	wet_line = layer->buf;
	ix = width + 1;
	for (y = 1; y < height - 1; y++) {
		wet_line += rs;
		for (x = 1; x < width - 1; x++) {
			if (wet_line[x].w > 0) {
				wet_pix_reduce(&wet_mix, &wet_old[ix],
					       1 - outflow[ix]);

				wet_pix_reduce(&wet_tmp, &wet_old[ix - rs],
					       flow_t[ix]);
				wet_pix_combine(&wet_mix, &wet_mix,
						&wet_tmp);
				wet_pix_reduce(&wet_tmp, &wet_old[ix + rs],
					       flow_b[ix]);
				wet_pix_combine(&wet_mix, &wet_mix,
						&wet_tmp);
				wet_pix_reduce(&wet_tmp, &wet_old[ix - 1],
					       flow_l[ix]);
				wet_pix_combine(&wet_mix, &wet_mix,
						&wet_tmp);
				wet_pix_reduce(&wet_tmp, &wet_old[ix + 1],
					       flow_r[ix]);
				wet_pix_combine(&wet_mix, &wet_mix,
						&wet_tmp);

				wet_pix_from_double(&wet_line[x],
						    &wet_mix);

#if 0
				if (ix % 3201 == 0)
					g_print("%f %f %f %f %f %f\n",
						outflow[ix],
						flow_t[ix],
						flow_b[ix],
						flow_l[ix],
						flow_r[ix], fluid[ix]);
#endif
			}
			ix++;
		}
		ix += 2;
	}

	g_free(flow_t);
	g_free(flow_b);
	g_free(flow_l);
	g_free(flow_r);
	g_free(fluid);
	g_free(outflow);
	g_free(wet_old);
}

void wet_dry(WetLayer * layer)
{
	int x, y;
	WetPix *wet_line = layer->buf;
	int width = layer->width;
	int height = layer->height;
	int rs = layer->rowstride;
	int w;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			w = wet_line[x].w;
			w -= 1;
			if (w > 0)
				wet_line[x].w = w;
			else
				wet_line[x].w = 0;

		}
		wet_line += rs;
	}
}

/* Move stuff from the upperlayer to the lower layer. This is filter-level stuff*/
void wet_adsorb(WetLayer * layer, WetLayer * adsorb)
{
	int x, y;
	WetPix *wet_line = layer->buf;
	WetPix *ads_line = adsorb->buf;
	int width = layer->width;
	int height = layer->height;
	int rs = layer->rowstride;
	double ads;
	WetPixDbl wet_top;
	WetPixDbl wet_bot;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/* do adsorption */
			if (wet_line[x].w == 0)
				continue;
			ads = 0.5 / MAX(wet_line[x].w, 1);

			wet_pix_to_double(&wet_top, &wet_line[x]);
			wet_pix_to_double(&wet_bot, &ads_line[x]);
			wet_pix_merge(&wet_bot, &wet_top, ads, &wet_bot);
			wet_pix_from_double(&ads_line[x], &wet_bot);
			wet_line[x].rd = wet_line[x].rd * (1 - ads);
			wet_line[x].rw = wet_line[x].rw * (1 - ads);
			wet_line[x].gd = wet_line[x].gd * (1 - ads);
			wet_line[x].gw = wet_line[x].gw * (1 - ads);
			wet_line[x].bd = wet_line[x].bd * (1 - ads);
			wet_line[x].bw = wet_line[x].bw * (1 - ads);
		}
		wet_line += rs;
		ads_line += rs;
	}
}
