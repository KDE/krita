/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <qglobal.h>

#include <kdebug.h>

/**
 * A number of often-used conversions between color models
 */

void rgb_to_hls(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b, float * h, float * l, float * s)
{
	int max, min, delta;

	max = QMAX(r, g);
	max = QMAX(max, b);
	
	min = QMIN(r, g);
	min = QMIN(min, b);

	delta = max - min;

	*l = (max + min) / 2;

	if (delta == 0) {
		// This is a gray, no chroma...
		*h = 0;
		*s = 0;
	}
	else {
		if ( *l < 0.5)
			*s = delta / ( max + min );
		else
			*s = delta / ( 2 - max - min );

		float delta_r, delta_g, delta_b;

		delta_r = ((( max - r ) / 6 ) + ( max / 2 )) / delta;
		delta_g = ((( max - g ) / 6 ) + ( max / 2 )) / delta;
		delta_b = ((( max - b ) / 6 ) + ( max / 2 )) / delta;

		if ( r == max )
			*h = delta_b - delta_g;
		else if ( g == max)
			*h = ( 1 / 3 ) + delta_r - delta_b;
		else if ( b == max)
			*h = ( 2 / 3 ) + delta_g - delta_r;

		if (*h < 0) *h += 1;
		if (*h > 1) *h += 1;
		
	}

	kdDebug() << " Converted rgb (" << r << ", " << g << ", " << b 
		  << ") to hls (" << *h << ", " << *l << ", " << *s << ")\n";
};

float hue_value(float n1, float n2, float hue)
{
	if (hue > 360 )
		hue = hue -360;
	else if (hue < 0 )
		hue = hue +360;
	if (hue < 60  )
		return n1 + (n2 - n1) * hue / 60;
	else if (hue < 180 )
		return n2;
	else if (hue < 240 )
		return n1 + (n2 - n1) * (240 - hue) / 60;
	else return n1;
};


void hls_to_rgb(float h, float l, float s, Q_UINT8 * r, Q_UINT8 * g, Q_UINT8 * b)
{
	float m1, m2;

	if (l <= 0.5 )
		m2 = l * ( 1 + s );
	else 
		m2 = l + s - l * s;

	m1 = 2 * l - m2;
	
	*r = (Q_UINT8)hue_value(m1, m2, h + 120);
	*g = (Q_UINT8)hue_value(m1, m2, h);
	*b = (Q_UINT8)hue_value(m1, m2, h - 120);

	kdDebug() << " Converted hls (" << h << ", " << l << ", " << s << ")"
		  << ") to rgb (" << *r << ", " << *g << ", " << *b << ")\n";

};
