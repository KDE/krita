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

#include "kis_color_conversions.h"

/**
 * A number of often-used conversions between color models
 */

void rgb_to_hsv(int R, int G, int B, int *H, int *S, int *V)
{
	unsigned int max = R;
	unsigned int min = R;
	unsigned char maxValue = 0; // r = 0, g = 1, b = 2

	// find maximum and minimum RGB values
	if(static_cast<unsigned int>(G) > max) {
		max = G;
		maxValue = 1;
	}
	
	if (static_cast<unsigned int>(B) > max)
	{
		max = B;
		maxValue = 2;
	}

	if(static_cast<unsigned int>(G) < min)
		min = G;
		
	if(static_cast<unsigned int>(B) < min )
		min = B;

	int delta = max - min;
	*V = max; // value
	*S = max ? (510 * delta + max) / ( 2 * max) : 0; // saturation
	
	// calc hue
	if(*S == 0)
		*H = -1; // undefined hue
	else
	{
		switch(maxValue)
		{
		case 0:  // red
			if(G >= B)
				*H = (120 * (G - B) + delta) / (2 * delta);
			else
				*H = (120 * (G - B + delta) + delta) / (2 * delta) + 300;
			break;
		case 1:  // green
			if(B > R)
				*H = 120 + (120 * (B - R) + delta) / (2 * delta);
			else
				*H = 60 + (120 * (B - R + delta) + delta) / (2 * delta);
			break;
		case 2:  // blue
			if(R > G)
				*H = 240 + (120 * (R - G) + delta) / (2 * delta);
			else
				*H = 180 + (120 * (R - G + delta) + delta) / (2 * delta);
			break;
		}
	}
}

void hsv_to_rgb(int H, int S, int V, int *R, int *G, int *B)
{
	*R = *G = *B = V;

	if (S != 0 && H != -1) { // chromatic

		if (H >= 360) {
			// angle > 360
			H %= 360;
		}

		unsigned int f = H % 60;
		H /= 60;
		unsigned int p = static_cast<unsigned int>(2*V*(255-S)+255)/510;
		unsigned int q, t;

		if (H & 1) {
			q = static_cast<unsigned int>(2 * V * (15300 - S * f) + 15300) / 30600;
			switch (H) {
			case 1:
				*R = static_cast<int>(q);
				*G = static_cast<int>(V);
				*B = static_cast<int>(p);
				break;
			case 3:
				*R = static_cast<int>(p);
				*G = static_cast<int>(q);
				*B = static_cast<int>(V);
				break;
			case 5:
				*R = static_cast<int>(V);
				*G = static_cast<int>(p);
				*B = static_cast<int>(q);
				break;
			}
		} else {
			t = static_cast<unsigned int>(2 * V * (15300 - (S * (60 - f))) + 15300) / 30600;
			switch (H) {
			case 0:
				*R = static_cast<int>(V);
				*G = static_cast<int>(t);
				*B = static_cast<int>(p);
				break;
			case 2:
				*R = static_cast<int>(p);
				*G = static_cast<int>(V);
				*B = static_cast<int>(t);
				break;
			case 4:
				*R = static_cast<int>(t);
				*G = static_cast<int>(p);
				*B = static_cast<int>(V);
				break;
			}
		}
	}
}

void rgb_to_hls(Q_UINT8 red, Q_UINT8 green, Q_UINT8 blue, float * hue, float * lightness, float * saturation)
{
	float r = red / 255.0;
	float g = green / 255.0;
	float b = blue / 255.0;
	float h, l, s;

	float max, min, delta;

	max = QMAX(r, g);
	max = QMAX(max, b);
	
	min = QMIN(r, g);
	min = QMIN(min, b);

	delta = max - min;

	l = (max + min) / 2;

	if (delta == 0) {
		// This is a gray, no chroma...
		h = 0;
		s = 0;
	}
	else {
		if ( l < 0.5)
			s = delta / ( max + min );
		else
			s = delta / ( 2 - max - min );

		float delta_r, delta_g, delta_b;

		delta_r = (( max - r ) / 6 ) / delta;
		delta_g = (( max - g ) / 6 ) / delta;
		delta_b = (( max - b ) / 6 ) / delta;

		if ( r == max )
			h = delta_b - delta_g;
		else if ( g == max)
			h = ( 1.0 / 3 ) + delta_r - delta_b;
		else if ( b == max)
			h = ( 2.0 / 3 ) + delta_g - delta_r;

		if (h < 0) h += 1;
		if (h > 1) h += 1;
		
	}

	*hue = h * 360;
	*saturation = s;
	*lightness = l;
};

float hue_value(float n1, float n2, float hue)
{
	if (hue > 360 )
		hue = hue -360;
	else if (hue < 0 )
		hue = hue +360;
	if (hue < 60  )
		return n1 + (((n2 - n1) * hue) / 60);
	else if (hue < 180 )
		return n2;
	else if (hue < 240 )
		return n1 + (((n2 - n1) * (240 - hue)) / 60);
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
	
	*r = (Q_UINT8)(hue_value(m1, m2, h + 120) * 255 + 0.5);
	*g = (Q_UINT8)(hue_value(m1, m2, h) * 255 + 0.5);
	*b = (Q_UINT8)(hue_value(m1, m2, h - 120) * 255 + 0.5);

	//kdDebug() << " Converted hls (" << h << ", " << l << ", " << s << ")"
	//	  << ") to rgb (" << *r << ", " << *g << ", " << *b << ")\n";

};

void rgb_to_hls(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b, int * h, int * l, int * s)
{
	float hue, saturation, lightness;

	rgb_to_hls(r, g, b, &hue, &lightness, &saturation);
	*h = (int)(hue + 0.5);
	*l = (int)(lightness * 255 + 0.5);
	*s = (int)(saturation * 255 + 0.5);
}

void hls_to_rgb(int h, int l, int s, Q_UINT8 * r, Q_UINT8 * g, Q_UINT8 * b)
{
	float hue = h;
	float lightness = l / 255.0;
	float saturation = s / 255.0;

	hls_to_rgb(hue, lightness, saturation, r, g, b);
}

