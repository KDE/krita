/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "lines.h"
#include <KoColor.h>
#include "kis_paint_device.h"

int Lines::gsfilter(float val){

int aliasMap[] = { 
	0xc7, 0xc6, 0xc2, 0xbc,
	0xb3, 0xa9, 0x9c, 0x8e,
	0x7f, 0x70, 0x62, 0x54,
	0x46, 0x3a, 0x2f, 0x25,
	0x1c, 0x15, 0x0e, 0x09,
	0x05, 0x03, 0x01, 0x00 };

	int ret;
	if (val < 0) val = -val;

if (val < 1/16) ret = aliasMap[0];
	else if (val < 2/16.0) ret = aliasMap[1];
	else if (val < 3/16.0) ret = aliasMap[2];
	else if (val < 4/16.0) ret = aliasMap[3];
	else if (val < 5/16.0) ret = aliasMap[4];
	else if (val < 6/16.0) ret = aliasMap[5];
	else if (val < 7/16.0) ret = aliasMap[6];
	else if (val < 8/16.0) ret = aliasMap[7];
	else if (val < 9/16.0) ret = aliasMap[8];
	else if (val < 10/16.0) ret =aliasMap[9];
	else if (val < 11/16.0) ret =aliasMap[10];
	else if (val < 12/16.0) ret =aliasMap[11];
	else if (val < 13/16.0) ret =aliasMap[12];
	else if (val < 14/16.0) ret =aliasMap[13];
	else if (val < 15/16.0) ret =aliasMap[14];
	else if (val < 16/16.0) ret =aliasMap[15];
	else if (val < 17/16.0) ret =aliasMap[16];
	else if (val < 18/16.0) ret =aliasMap[17];
	else if (val < 19/16.0) ret =aliasMap[18];
	else if (val < 20/16.0) ret =aliasMap[19];
	else if (val < 21/16.0) ret =aliasMap[20];
	else if (val < 22/16.0) ret =aliasMap[21];
	else if (val < 23/16.0) ret =aliasMap[22];
	else ret = aliasMap[23];

	return ret;
}


void Lines::drawGSLine(KisPaintDeviceSP image, int x0, int y0, int x1, int y1, int w1, int w2, const KoColor &color)
{
Q_ASSERT(image);
int da, incrEa, incrNEa, incrNa, incrSEa, incrSa;
int db, incrEb, incrNEb, incrNb, incrSEb, incrSb;
int d, incrE, incrNE, incrN, incrSE, incrS;
int xa, ya, xb, yb, x, y ;
int tp0, tn0, tp1, tn1;
int Aa; double Ba, Ca;
int Ab; double Bb, Cb;

int dir = 0;


//   if (w1!=1) w1--;
//   if (w2!=1) w2--;

if ( x1 < x0 ) // sort so x0<x1
{
	int xt, yt, wt;
	xt = x0;       x0 = x1;       x1 = xt;
	yt = y0;       y0 = y1;       y1 = yt;
	wt = w2;       w2 = w1;       w1 = wt;
}

tp0 = w1/2;
tn0 = w1/2;
if ( w1%2==0 ) // even width w1
	tn0--;

tp1 = w2/2;
tn1 = w2/2;
if ( w2%2==0 ) // even width w2
	tn1--;

int dx = x1 - x0; // run
int dy = y1 - y0; // rise
int ady;
if ( dy < 0 ) ady = -dy;
else ady = dy;
int adx;
if ( dx < 0 ) adx = -dx;
else adx = dx;

int x0a, y0a, x1a, y1a, x0b, y0b, x1b, y1b;

if ( adx > ady ) // horizontalish
{
	x0a = x0;   y0a = y0 - tn0;
	x0b = x0;   y0b = y0 + tp0;
	x1a = x1;   y1a = y1 - tn1;
	x1b = x1;   y1b = y1 + tp1;
}
else
{
	x0a = x0 - tn0;   y0a = y0;
	x0b = x0 + tp0;   y0b = y0;
	x1a = x1 - tn1;   y1a = y1;
	x1b = x1 + tp1;   y1b = y1;
}

int dxa, dxb, dya, dyb, adya, adyb;
dxa = x1a - x0a; // run of a
dya = y1a - y0a; // rise of a
dxb = x1b - x0b; // run of b
dyb = y1b - y0b; // rise of b

if ( dya < 0 ) adya = -dya;
else adya = dya;
if ( dyb < 0 ) adyb = -dyb;
else adyb = dyb;


if ( dy > 0 ) // y1>y0 -- line goes right
{
	if ( dx >= dy ) dir = 1; // more run than rise
	else dir = 2;          // less run than rise
}
else  // y1<=y0 -- line goes right
{
	if ( dx >= -1*dy ) dir = 3;  // more run than rise
	else dir = 4;          // less run than rise
}

float perc; int intensity;
KoColor lineColor( color );

switch ( dir )
{
case 1:
	Aa = 0;
	Ba = 1.0/ ( 2.0 * sqrt ( dxa *dxa + dya * dya ) );
	Ca = 2.0 * dxa * Ba;

	Ab = 0;
	Bb = 1.0/ ( 2.0 * sqrt ( dxb *dxb + dyb * dyb ) );
	Cb = 2.0 * dxb * Bb;

	da = 2*dya-dxa;
	incrEa  = 2*dya;
	incrNEa = 2* ( dya-dxa );
	ya = y0a;

	db = 2*dyb-dxb;
	incrEb  = 2*dyb;
	incrNEb = 2* ( dyb-dxb );
	yb = y0b;

	d = 2*dy-dx;
	incrE  = 2*dy;
	incrNE = 2* ( dy-dx );
	x = x0;


	// line a ------------------------------------------------
	intensity = gsfilter ( 0 );
	lineColor.setOpacity ( intensity );
	image->setPixel ( x,ya,lineColor );

	intensity = gsfilter ( Ca - Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( x,ya+1,lineColor );

	intensity = gsfilter ( Ca + Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( x,ya+1,lineColor );

	//--------------------------------------------------------
	// line b ------------------------------------------------
	//if (!(w1==1 && w2==1))
	{

		intensity = gsfilter ( Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,yb,lineColor );

		intensity = gsfilter ( Cb - Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,yb+1,lineColor );

		intensity = gsfilter ( Cb + Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,yb-1,lineColor );

	}
	//--------------------------------------------------------

	while ( x < x1 )
	{
		if ( da <= 0 )
		{
			Aa = da + dxa;
			da += incrEa;
		}
		else
		{
			Aa = da - dxa;
			da += incrNEa;
			ya++;
		}

		if ( db <= 0 )
		{
			Ab = db + dxb;
			db += incrEb;
		}
		else
		{
			Ab = db - dxb;
			db += incrNEb;
			yb++;
		}

		if ( d <= 0 )
		{
			d += incrE;
			x++;
		}
		else
		{
			d += incrNE;
			x++;
		}

		perc = ( float ) ( x-x0 ) / ( float ) adx;

		// line a ------------------------------------------------
		intensity = gsfilter ( Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,ya,lineColor );

		intensity = gsfilter ( Ca - Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,ya+1,lineColor );

		intensity = gsfilter ( Ca + Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,ya-1,lineColor );

		//--------------------------------------------------------
		// line b ------------------------------------------------
		//if (!(w1==1 && w2==1))
		{

			intensity = gsfilter ( Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( x,yb,lineColor );

			intensity = gsfilter ( Cb - Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( x,yb+1,lineColor );

			intensity = gsfilter ( Cb + Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( x,yb-1,lineColor );

		}
		//--------------------------------------------------------
		lineColor.setOpacity ( (int)(255*perc) ); // 255*perc is cool, gradient color
		// fill remaining pixels between current xfa,xfb
		if ( ! ( w1==1 && w2==1 ) )
		{
			if ( ya<yb )
				for ( int i=ya; i<=yb; i++ )
					image->setPixel ( x,i,lineColor );
			else
				for ( int i=yb; i<=ya; i++ )
					image->setPixel ( x,i,lineColor );
		}

	}

	break;

case 2:
	Aa = 0;
	Ba = 1.0/ ( 2.0 * sqrt ( dxa *dxa + dya * dya ) );
	Ca = 2.0 * dxa * Ba;

	Ab = 0;
	Bb = 1.0/ ( 2.0 * sqrt ( dxb *dxb + dyb * dyb ) );
	Cb = 2.0 * dxb * Bb;

	da = dya-2*dxa;
	incrNa  = -2*dxa;
	incrNEa = 2* ( dya-dxa );
	xa = x0a;

	db = dyb-2*dxb;
	incrNb  = -2*dxb;
	incrNEb = 2* ( dyb-dxb );
	xb = x0b;

	d = dy-2*dx;
	incrN  = -2*dx;
	incrNE = 2* ( dy-dx );
	y = y0;

	// line a ------------------------------------------------
	intensity = gsfilter ( 0 );
	lineColor.setOpacity ( intensity );
	image->setPixel ( xa,y,lineColor );

	intensity = gsfilter ( Ca - Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( xa+1,y,lineColor );

	intensity = gsfilter ( Ca + Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( xa-1,y,lineColor );

	//--------------------------------------------------------
	// line b ------------------------------------------------
	//if (!(w1==1 && w2==1))
	{

		intensity = gsfilter ( Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xb,y,lineColor );

		intensity = gsfilter ( Cb - Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xb+1,y,lineColor );

		intensity = gsfilter ( Cb + Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xb-1,y,lineColor );
	}
	//--------------------------------------------------------

	while ( y < y1 )
	{
		if ( da <= 0 )
		{
			Aa = da + dya;
			da += incrNEa;
			xa++;
		}
		else
		{
			Aa = da - dya;
			da += incrNa;
		}

		if ( db <= 0 )
		{
			Ab = db + dyb;
			db += incrNEb;
			xb++;
		}
		else
		{
			Ab = db - dyb;
			db += incrNb;
		}

		if ( d <= 0 )
		{
			d += incrNE;
			y++;
		}
		else
		{
			d += incrN;
			y++;
		}

		perc = ( float ) ( y-y0 ) / ( float ) ady;
		/*c3.r = perc*c2.r + (1 - perc)*c1.r;
		c3.g = perc*c2.g + (1 - perc)*c1.g;
		c3.b = perc*c2.b + (1 - perc)*c1.b;*/


		// line a ------------------------------------------------
		intensity = gsfilter ( Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xa,y,lineColor );

		intensity = gsfilter ( Ca - Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xa+1,y,lineColor );

		intensity = gsfilter ( Ca + Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xa-1,y,lineColor );

		//--------------------------------------------------------
		// line b ------------------------------------------------
		//if (!(w1==1 && w2==1))
		{
			intensity = gsfilter ( Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( xb,y,lineColor );

			intensity = gsfilter ( Cb - Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( xb+1,y,lineColor );

			intensity = gsfilter ( Cb + Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( xb-1,y,lineColor );
		}
		//--------------------------------------------------------

		lineColor.setOpacity ( 255 );
		// fill remaining pixels between current xfa,xfb
		if ( ! ( w1==1 && w2==1 ) )
		{
			if ( xa<xb )
				for ( int i=xa; i<=xb; i++ )
					image->setPixel ( i,y,lineColor );

			else
				for ( int i=xb; i<=xa; i++ )
					image->setPixel ( i,y,lineColor );
		}
	}

	break;

case 3:
	Aa = 0;
	Ba = 1.0/ ( 2.0 * sqrt ( dxa *dxa + dya * dya ) );
	Ca = 2.0 * dxa * Ba;

	Ab = 0;
	Bb = 1.0/ ( 2.0 * sqrt ( dxb *dxb + dyb * dyb ) );
	Cb = 2.0 * dxb * Bb;

	da = 2*dya+dxa;
	incrEa  = 2*dya;
	incrSEa = 2* ( dya+dxa );
	ya = y0a;

	db = 2*dyb+dxb;
	incrEb  = 2*dyb;
	incrSEb = 2* ( dyb+dxb );
	yb = y0b;

	d = 2*dy+dx;
	incrE  = 2*dy;
	incrSE = 2* ( dy+dx );
	x = x0;

	// line a ------------------------------------------------
	intensity = gsfilter ( 0 );
	lineColor.setOpacity ( intensity );
	image->setPixel ( x,ya,lineColor );

	intensity = gsfilter ( Ca - Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( x,ya+1,lineColor );

	intensity = gsfilter ( Ca + Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( x,ya-1,lineColor );


	//--------------------------------------------------------
	// line b ------------------------------------------------
	//if (!(w1==1 && w2==1))
	{
		intensity = gsfilter ( Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,yb,lineColor );

		intensity = gsfilter ( Cb - Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,yb+1,lineColor );

		intensity = gsfilter ( Cb + Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,yb-1,lineColor );
	}
	//--------------------------------------------------------

	while ( x < x1 )
	{
		if ( da <= 0 )
		{
			Aa = da + dxa;
			da += incrSEa;
			ya--;
		}
		else
		{
			Aa = da - dxa;
			da += incrEa;
		}

		if ( db <= 0 )
		{
			Ab = db + dxb;
			db += incrSEb;
			yb--;
		}
		else
		{
			Ab = db - dxb;
			db += incrEb;
		}

		if ( d <= 0 )
		{
			d += incrSE;
			x++;
		}
		else
		{
			d += incrE;
			x++;
		}

		perc = ( float ) ( x-x0 ) / ( float ) adx;

		// line a ------------------------------------------------
		intensity = gsfilter ( Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,ya,lineColor );

		intensity = gsfilter ( Ca - Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,ya+1,lineColor );

		intensity = gsfilter ( Ca + Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( x,ya-1,lineColor );

		//--------------------------------------------------------
		// line b ------------------------------------------------
		//if (!(w1==1 && w2==1))
		{
			intensity = gsfilter ( Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( x,yb,lineColor );

			intensity = gsfilter ( Cb - Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( x,yb+1,lineColor );


			intensity = gsfilter ( Cb + Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( x,yb-1,lineColor );
		}
		//--------------------------------------------------------
		lineColor.setOpacity ( (int)(255*perc) );
		// fill remaining pixels between current xfa,xfb
		if ( ! ( w1==1 && w2==1 ) )
		{
			if ( ya<yb )
				for ( int i=ya; i<=yb; i++ )
					image->setPixel ( x,i,lineColor );

			else
				for ( int i=yb; i<=ya; i++ )
					image->setPixel ( x,i,lineColor );
		}
	}

	break;

case 4:
	Aa = 0;
	Ba = 1.0/ ( 2.0 * sqrt ( dxa *dxa + dya * dya ) );
	Ca = 2.0 * dxa * Ba;

	Ab = 0;
	Bb = 1.0/ ( 2.0 * sqrt ( dxb *dxb + dyb * dyb ) );
	Cb = 2.0 * dxb * Bb;

	da = dya+2*dxa;
	incrSEa = 2* ( dya+dxa );
	incrSa  = 2*dxa;
	xa = x0a;

	db = dyb+2*dxb;
	incrSEb = 2* ( dyb+dxb );
	incrSb  = 2*dxb;
	xb = x0b;

	d = dy+2*dx;
	incrSE = 2* ( dy+dx );
	incrS  = 2*dx;
	y = y0;


	// line a ------------------------------------------------
	intensity = gsfilter ( 0 );
	lineColor.setOpacity ( intensity );
	image->setPixel ( xa,y,lineColor );

	intensity = gsfilter ( Ca - Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( xa+1,y,lineColor );

	intensity = gsfilter ( Ca + Aa*Ba );
	lineColor.setOpacity ( intensity );
	image->setPixel ( xa-1,y,lineColor );


	//--------------------------------------------------------
	// line b ------------------------------------------------
	//if (!(w1==1 && w2==1))
	{
		intensity = gsfilter ( Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xb,y,lineColor );

		intensity = gsfilter ( Cb - Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xb+1,y,lineColor );


		intensity = gsfilter ( Cb + Ab*Bb );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xb-1,y,lineColor );

	}
	//--------------------------------------------------------

	while ( y > y1 )
	{
		if ( da <= 0 )
		{
			Aa = da + dya;
			da += incrSa;
		}
		else
		{
			Aa = da - dya;
			da += incrSEa;
			xa++;
		}

		if ( db <= 0 )
		{
			Ab = db + dyb;
			db += incrSb;
		}
		else
		{
			Ab = db - dyb;
			db += incrSEb;
			xb++;
		}

		if ( d <= 0 )
		{
			d += incrS;
			y--;
		}
		else
		{
			d += incrSE;
			y--;
		}

		perc = ( float ) ( y0-y ) / ( float ) ady;

		// line a ------------------------------------------------
		intensity = gsfilter ( Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xa,y,lineColor );

		intensity = gsfilter ( Ca - Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xa+1,y,lineColor );


		intensity = gsfilter ( Ca + Aa*Ba );
		lineColor.setOpacity ( intensity );
		image->setPixel ( xa-1,y,lineColor );


		//--------------------------------------------------------
		// line b ------------------------------------------------
		//if (!(w1==1 && w2==1))
		{
			intensity = gsfilter ( Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( xb,y,lineColor );

			intensity = gsfilter ( Cb - Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( xb+1,y,lineColor );

			intensity = gsfilter ( Cb + Ab*Bb );
			lineColor.setOpacity ( intensity );
			image->setPixel ( xb-1,y,lineColor );
		}
		//--------------------------------------------------------
		lineColor.setOpacity ( 255 );
		// fill remaining pixels between current xfa,xfb
		if ( ! ( w1==1 && w2==1 ) )
		{
			if ( xa<xb )
				for ( int i=xa; i<=xb; i++ )
					image->setPixel ( i,y,lineColor );
			else
				for ( int i=xb; i<=xa; i++ )
					image->setPixel ( i,y,lineColor );
		}
	}

	break;
}
}


void Lines::drawLine(int x0,int y0,int x1,int y1,KoColor color){
	Q_UNUSED(x0);
	Q_UNUSED(y0);
	Q_UNUSED(x1);
	Q_UNUSED(y1);
}




