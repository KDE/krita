
#include <stdlib.h>
#include <math.h>
#include "wetpix.h"
#include "wetphysics.h"
#include "wetpaint.h"

/* This function is not entirely satisfactory - the compositing is basically
   opaque, and it really should do wet compositing. */
void
wet_dab(WetLayer * layer,
	WetPix * paint,
	double x, double y, double r, double pressure, double strength)
{
	double r_fringe;
	int x0, y0;
	int x1, y1;
	WetPix *wet_line;
	int xp, yp;
	double xx, yy, rr;
	double eff_height;
	double press, contact;
	WetPixDbl wet_tmp, wet_tmp2;

	r_fringe = r + 1;
	x0 = floor(x - r_fringe);
	y0 = floor(y - r_fringe);
	x1 = ceil(x + r_fringe);
	y1 = ceil(y + r_fringe);
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 >= layer->width)
		x1 = layer->width;
	if (y1 >= layer->height)
		y1 = layer->height;

	wet_line = layer->buf + y0 * layer->rowstride;
	for (yp = y0; yp < y1; yp++) {
		yy = (yp + 0.5 - y);
		yy *= yy;
		for (xp = x0; xp < x1; xp++) {
			xx = (xp + 0.5 - x);
			xx *= xx;
			rr = yy + xx;
			if (rr < r * r)
				press = pressure * 0.25;
			else
				press = -1;
			eff_height =
			    (wet_line[xp].h + wet_line[xp].w -
			     192) * (1.0 / 255);
			contact = (press + eff_height) * 0.2;
			if (contact > 0.5)
				contact =
				    1 - 0.5 * exp(-2.0 * contact - 1);
			if (contact > 0.0001) {
				int v;
				double rnd = rand() * (1.0 / RAND_MAX);

				v = wet_line[xp].rd;
				wet_line[xp].rd =
				    floor(v +
					  (paint->rd * strength -
					   v) * contact + rnd);
				v = wet_line[xp].rw;
				wet_line[xp].rw =
				    floor(v +
					  (paint->rw * strength -
					   v) * contact + rnd);
				v = wet_line[xp].gd;
				wet_line[xp].gd =
				    floor(v +
					  (paint->gd * strength -
					   v) * contact + rnd);
				v = wet_line[xp].gw;
				wet_line[xp].gw =
				    floor(v +
					  (paint->gw * strength -
					   v) * contact + rnd);
				v = wet_line[xp].bd;
				wet_line[xp].bd =
				    floor(v +
					  (paint->bd * strength -
					   v) * contact + rnd);
				v = wet_line[xp].bw;
				wet_line[xp].bw =
				    floor(v +
					  (paint->bw * strength -
					   v) * contact + rnd);
				v = wet_line[xp].w;
				wet_line[xp].w =
				    floor(v + (paint->w - v) * contact +
					  rnd);

			}
		}
		wet_line += layer->rowstride;
	}
}
