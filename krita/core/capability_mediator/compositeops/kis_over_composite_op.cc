/*
 *  Copyright (c) 2004, Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; wit1out even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qstring.h>

#include "kis_global.h"
#include "kis_over_composite_op.h"

KisOverCompositeOp::KisOverCompositeOp() : super()
{
}

KisOverCompositeOp::KisOverCompositeOp(QString & label,
				       QString & description) 
	: super( label , description )
{
}

KisOverCompositeOp::~KisOverCompositeOp()
{
}


void KisOverCompositeOp::composite(Q_INT32 stride,
				   QUANTUM *dst, 
				   Q_INT32 dststride,
				   QUANTUM *src, 
				   Q_INT32 srcstride,
				   Q_INT32 rows, 
				   Q_INT32 cols, 
				   QUANTUM opacity) const
{
	QUANTUM *d;
	QUANTUM *s;
	QUANTUM alpha;
	QUANTUM invAlpha;
	Q_INT32 i;

	if (opacity == OPACITY_TRANSPARENT) 
		return;

	if (opacity != OPACITY_OPAQUE) {

			while (rows-- > 0) {
				d = dst;
				s = src;

				for (i = cols; i > 0; i--, d += stride, s += stride) {
					if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
						continue;

					alpha = (s[PIXEL_ALPHA] * opacity) / QUANTUM_MAX;
					invAlpha = QUANTUM_MAX - alpha;

					d[PIXEL_RED]   = (d[PIXEL_RED]   * invAlpha + s[PIXEL_RED]   * alpha) / QUANTUM_MAX;
					d[PIXEL_GREEN] = (d[PIXEL_GREEN] * invAlpha + s[PIXEL_GREEN] * alpha) / QUANTUM_MAX;
					d[PIXEL_BLUE]  = (d[PIXEL_BLUE]  * invAlpha + s[PIXEL_BLUE]  * alpha) / QUANTUM_MAX;
					alpha = (d[PIXEL_ALPHA] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
					d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
				}

				dst += dststride;
				src += srcstride;
			}
	}
	else {

		while (rows-- > 0) {
			d = dst;
			s = src;
		
			for (i = cols; i > 0; i--, d += stride, s += stride) {
				if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
					continue;
			
				if (d[PIXEL_ALPHA] == OPACITY_TRANSPARENT || (d[PIXEL_ALPHA] == OPACITY_OPAQUE && s[PIXEL_ALPHA] == OPACITY_OPAQUE)) {
					memcpy(d, s, stride * sizeof(QUANTUM));
					continue;
				}
				invAlpha = QUANTUM_MAX - s[PIXEL_ALPHA];
				alpha    = (d[PIXEL_ALPHA] * invAlpha + s[PIXEL_ALPHA]) / QUANTUM_MAX;

				d[PIXEL_RED]   = (d[PIXEL_RED]   * invAlpha + s[PIXEL_RED]   * s[PIXEL_ALPHA]) / QUANTUM_MAX;
				d[PIXEL_GREEN] = (d[PIXEL_GREEN] * invAlpha + s[PIXEL_GREEN] * s[PIXEL_ALPHA]) / QUANTUM_MAX;
				d[PIXEL_BLUE]  = (d[PIXEL_BLUE]  * invAlpha + s[PIXEL_BLUE]  * s[PIXEL_ALPHA]) / QUANTUM_MAX;
				d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
			}
		
			dst += dststride;
			src += srcstride;
		}
	}
}
