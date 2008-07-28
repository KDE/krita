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
#include <cmath>
void Lines::drawLine(KisPaintDeviceSP dev, int x0,int y0,int x1,int y1,const KoColor &color){
	drawThickLine(dev,x0,y0,x1,y1,color,color,1,1);
}


void Lines::drawDDALine(KisPaintDeviceSP image, int x1, int y1, int x2, int y2,const KoColor &color){
	Q_ASSERT(image);
// Width and height of the line
	int xd = (x2 - x1);
	int yd = (y2 - y1);

int x;
int y;
float fx = (x = x1);
float fy = (y = y1);
float m = (float)yd/(float)xd;

if ( fabs(m) > 1 )
{
	int incr;
	if ( yd > 0 )
	{
		m = 1.0f/m;
		incr = 1;
	}
	else
	{
		m = -1.0f/m;
		incr = -1;
	}
	while ( y!=y2 )
	{
		fx = fx + m;
		y = y + incr;
		x = (int)(fx + 0.5f);
		image->setPixel(x,y,color);
	}
}else
{
	int incr;
	if ( xd > 0 )
	{
		incr = 1;
	}else
	{
		incr = -1;
		m = -m;
	}
	while ( x!=x2 )
	{
		fy= fy + m;
		x = x + incr;
		y = (int)(fy + 0.5f);
		image->setPixel ( x,y,color );
	}
}

}

void Lines::drawDDAALine(KisPaintDeviceSP image, int x1, int y1, int x2, int y2,const KoColor &color){
	Q_ASSERT(image);

	KoColor mycolor(color);
// Width and height of the line
	int xd = (x2 - x1);
	int yd = (y2 - y1);

int x;
int y;
float fx = (x = x1);
float fy = (y = y1);
float m = (float)yd/(float)xd;

if ( fabs(m) > 1 )
{
	int incr;
	if ( yd > 0 )
	{
		m = 1.0f/m;
		incr = 1;
	}
	else
	{
		m = -1.0f/m;
		incr = -1;
	}
	while ( y!=y2 )
	{
		fx = fx + m;
		y = y + incr;
		
		x = (int)(fx + 0.5f);
		float br1 = int(fx+1) - fx;
		float br2 = fx - (int)fx;

		mycolor.setOpacity( (int)(255*br1) );
		image->setPixel(x,y, mycolor);

		mycolor.setOpacity( (int)(255*br2) );
		image->setPixel(x+1,y, mycolor);
		
	}
}else
{
	int incr;
	if ( xd > 0 )
	{
		incr = 1;
	}else
	{
		incr = -1;
		m = -m;
	}
	while ( x!=x2 )
	{
		fy= fy + m;
		x = x + incr;
		y = (int)(fy + 0.5f);

		float br1 = int(fy+1) - fy;
		float br2 = fy - (int)fy;

		mycolor.setOpacity( (int)(255*br1) );
		image->setPixel(x,y,mycolor);
		
		mycolor.setOpacity( (int)(255*br2) );
		image->setPixel(x,y+1,mycolor);
	}
}

}

float inline Lines::frac(float value)
{
	float tmp = 0;
	return modff(value , &tmp);
}

float inline Lines::invertFrac(float value){
	float tmp = 0;
	return 1.0f - modff(value , &tmp);
}

void Lines::drawWuLine ( KisPaintDeviceSP dev, float x1, float y1, float x2, float y2, const KoColor &color )
{
KoColor lineColor ( color );

float grad, xd, yd,
	xgap,ygap, xend, yend, yf,xf,
	brightness1, brightness2;

int ix1, ix2, iy1, iy2;
int c1, c2;
const float MaxPixelValue = 255.0f;

// gradient of line
xd = ( x2 - x1 );
yd = ( y2 - y1 );

if (yd == 0){
	/* Horizontal line */
	int incr = (x1 < x2) ? 1 : -1;
	ix1 = (int)x1;
	ix2 = (int)x2;
	iy1 = (int)y1;
	while(ix1!=ix2){
		ix1 = ix1 + incr;
		dev->setPixel(ix1,iy1, color);
	}
	return;
}

if (xd == 0){
	/* Vertical line */
	int incr = (y1 < y2) ? 1 : -1;
	iy1 = (int)y1;
	iy2 = (int)y2;
	ix1 = (int)x1;
	while(iy1!=iy2){
		iy1 = iy1 + incr;
		dev->setPixel(ix1,iy1, color);
	}
	return;
}


if (fabs( xd ) > fabs( yd ) ){
	// horizontal line
	// line have to be paint from left to right
	if ( x1 > x2 )
	{
		float tmp;
		tmp=x1;x1=x2;x2=tmp;
		tmp=y1;y1=y2;y2=tmp;
		xd = ( x2 - x1 );
		yd = ( y2 - y1 );
	}
	grad = yd/xd;
	// nearest X,Y interger coordinates
	xend = static_cast<int>( x1+0.5f );
	yend = y1 + grad * ( xend - x1 );

	xgap = invertFrac ( x1 + 0.5f );

	ix1 = static_cast<int> ( xend );
	iy1 = static_cast<int> ( yend );

	// calc the intensity of the other end point pixel pair.
	brightness1 = invertFrac ( yend ) * xgap;
	brightness2 =       frac ( yend ) * xgap;

	c1 = ( int ) ( brightness1 * MaxPixelValue );
	c2 = ( int ) ( brightness2 * MaxPixelValue );

	lineColor.setOpacity ( c1 );
	dev->setPixel ( ix1, iy1, lineColor );

	lineColor.setOpacity ( c2 );
	dev->setPixel ( ix1, iy1+1, lineColor );

	// calc first Y-intersection for main loop
	yf = yend+grad;

	xend = trunc ( x2+0.5f );
	yend = y2 + grad * ( xend - x2 );

	xgap = invertFrac ( x2-0.5f );

	ix2 = static_cast<int> ( xend );
	iy2 = static_cast<int> ( yend );

	brightness1 = invertFrac ( yend ) * xgap;
	brightness2 =    frac ( yend ) * xgap;

	c1 = ( int ) ( brightness1 * MaxPixelValue );
	c2 = ( int ) ( brightness2 * MaxPixelValue );

	lineColor.setOpacity ( c1 );
	dev->setPixel ( ix2,iy2, lineColor );

	lineColor.setOpacity ( c1 );
	dev->setPixel ( ix2,iy2+1, lineColor );

	// main loop
	for ( int x = ix1+1; x <= ix2-1; x++ )
	{
		brightness1 = invertFrac ( yf );
		brightness2 =    frac ( yf );
		c1 = ( int ) ( brightness1 * MaxPixelValue );
		c2 = ( int ) ( brightness2 * MaxPixelValue );

		lineColor.setOpacity ( c1 );
		dev->setPixel ( x,int ( yf ),lineColor );
		lineColor.setOpacity ( c2 );
		dev->setPixel ( x,int ( yf ) + 1, lineColor );

		yf = yf + grad;
	}
}
else{
//vertical
// line have to be paint from left to right
	if ( y1 > y2 )
	{
		float tmp;
		tmp=x1;x1=x2;x2=tmp;
		tmp=y1;y1=y2;y2=tmp;
		xd = ( x2 - x1 );
		yd = ( y2 - y1 );
	}

	grad = xd/yd;

	// nearest X,Y interger coordinates
	yend = static_cast<int> ( y1+0.5f );
	xend = x1 + grad * ( yend - y1 );

	ygap = invertFrac ( y1 + 0.5f );

	ix1 = static_cast<int> ( xend );
	iy1 = static_cast<int> ( yend );

	// calc the intensity of the other end point pixel pair.
	brightness1 = invertFrac ( xend ) * ygap;
	brightness2 =       frac ( xend ) * ygap;

	c1 = ( int ) ( brightness1 * MaxPixelValue );
	c2 = ( int ) ( brightness2 * MaxPixelValue );

	lineColor.setOpacity ( c1 );
	dev->setPixel ( ix1, iy1, lineColor );

	lineColor.setOpacity ( c2 );
	dev->setPixel ( ix1 + 1, iy1, lineColor );

	// calc first Y-intersection for main loop
	xf = xend+grad;

	yend = trunc ( y2+0.5f );
	xend = x2 + grad * ( yend - y2 );

	ygap = invertFrac ( y2-0.5f );

	ix2 = static_cast<int> ( xend );
	iy2 = static_cast<int> ( yend );

	brightness1 = invertFrac ( xend ) * ygap;
	brightness2 =    frac ( xend ) * ygap;

	c1 = ( int ) ( brightness1 * MaxPixelValue );
	c2 = ( int ) ( brightness2 * MaxPixelValue );

	lineColor.setOpacity ( c1 );
	dev->setPixel ( ix2,iy2, lineColor );

	lineColor.setOpacity ( c1 );
	dev->setPixel ( ix2+1,iy2, lineColor );

	// main loop
	for ( int y = iy1+1; y <= iy2-1; y++ )
	{
		brightness1 = invertFrac ( xf );
		brightness2 =    frac ( xf );
		c1 = ( int ) ( brightness1 * MaxPixelValue );
		c2 = ( int ) ( brightness2 * MaxPixelValue );

		lineColor.setOpacity ( c1 );
		dev->setPixel ( int (xf ),y,lineColor );

		lineColor.setOpacity ( c2 );
		dev->setPixel ( int (xf )+1,y, lineColor );
		xf = xf + grad;
	}
}//end-of-else

}

// cool wu lines with thickness support
void Lines::drawThickLine ( KisPaintDeviceSP dev, int x0, int y0, int x1, int y1,
                            const KoColor color1, const KoColor color2, int w1, int w2 )
{
	KoColor c1 ( color1 );
	KoColor c2 ( color2 );
	KoColor c3 ( color1 );
	KoColor col1 ( c1 );
	KoColor col2 ( c1 );

	float grada, gradb, dxa, dxb, dya, dyb, adya, adyb, fraca, fracb,
	xfa, yfa, xfb, yfb, b1a, b2a, b1b, b2b, dx, dy;
	int x, y, ix1, ix2, iy1, iy2;
	KoColor pix;
	int x0a, y0a, x1a, y1a, x0b, y0b, x1b, y1b;
	int tp0, tn0, tp1, tn1;
	int horizontal = 0;
	float opacity = 0.0;

	tp0 = w1/2;
	tn0 = w1/2;
	if ( w1%2==0 ) // even width w1
		tn0--;

	tp1 = w2/2;
	tn1 = w2/2;
	if ( w2%2==0 ) // even width w2
		tn1--;

	dx = x1 - x0; // run of general line
	dy = y1 - y0; // rise of general line

	if ( dy < 0 ) dy = -dy;
	if ( dx < 0 ) dx = -dx;

	if ( dx > dy ) // horizontalish
	{
		horizontal=1;
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

	if ( horizontal ) // draw endpoints
	{
		for ( int i=y0a; i<=y0b; i++ )
			dev->setPixel ( x0,i,c1 );
		for ( int i=y1a; i<=y1b; i++ )
			dev->setPixel ( x1,i,c1 );
	}
	else
	{
		for ( int i=x0a; i<=x0b; i++ )
			dev->setPixel ( i,y0,c1 );
		for ( int i=x1a; i<=x1b; i++ )
			dev->setPixel ( i,y1,c1 );
	}

	//antialias endpoints
	if ( x1!=x0 && y1!=y0 )
	{
		if ( horizontal )
		{
			dev->pixel ( x0a,y0a-1,&pix );
			opacity = .25*c1.opacity() + ( 1-.25 ) *pix.opacity();
			col1.setOpacity ( static_cast<int> ( opacity ) );
			dev->setPixel ( x0a, y0a-1, col1 );

			dev->pixel ( x1b,y1b+1,&pix );
			opacity = .25*c2.opacity() + ( 1-.25 ) *pix.opacity();
			col1.setOpacity ( static_cast<int> ( opacity ) );
			dev->setPixel ( x1b, y1b+1, col1 );
		}
		else
		{
			dev->pixel ( x0a-1,y0a,&pix );
			opacity = .25*c1.opacity() + ( 1-.25 ) *pix.opacity();
			col1.setOpacity ( static_cast<int> ( opacity ) );
			dev->setPixel ( x0a-1, y0a, col1 );

			dev->pixel ( x1b+1,y1b,&pix );
			opacity = .25*c2.opacity() + ( 1-.25 ) *pix.opacity();
			col1.setOpacity ( static_cast<int> ( opacity ) );
			dev->setPixel ( x1b+1, y1b, col1 );
		}
	}

	dxa = x1a - x0a; // run of a
	dya = y1a - y0a; // rise of a
	dxb = x1b - x0b; // run of b
	dyb = y1b - y0b; // rise of b

	if ( dya < 0 ) adya = -dya;
	else adya = dya;
	if ( dyb < 0 ) adyb = -dyb;
	else adyb = dyb;


	if ( horizontal ) // horizontal-ish lines
	{
		if ( x1 < x0 )
		{
			int xt, yt, wt;
			KoColor tmp;
			xt = x1a;     x1a = x0a;    x0a = xt;
			yt = y1a;     y1a = y0a;    y0a = yt;
			xt = x1b;     x1b = x0b;    x0b = xt;
			yt = y1b;     y1b = y0b;    y0b = yt;
			xt = x1;      x1 = x0;      x0 = xt;
			yt = y1;      y1 = y0;      y0 = yt;

			tmp = c1; c1= c2; c2 = tmp;
			wt = w1;      w1 = w2;      w2 = wt;
		}

		grada = dya/dxa;
		gradb = dyb/dxb;

		ix1 = x0;   iy1 = y0;
		ix2 = x1;   iy2 = y1;

		yfa = y0a + grada;
		yfb = y0b + gradb;

		for ( x = ix1+1; x <= ix2-1; x++ )
		{
			fraca = yfa - int ( yfa );
			b1a = 1 - fraca;
			b2a = fraca;

			fracb = yfb - int ( yfb );
			b1b = 1 - fracb;
			b2b = fracb;

			// color first pixel of bottom line
			opacity = ( ( x-ix1 ) /dx ) *c2.opacity() + ( 1 - ( x-ix1 ) /dx ) *c1.opacity();
			c3.setOpacity ( static_cast<int> ( opacity ) );

			dev->pixel ( x, (int)yfa, &pix );
			opacity = b1a*c3.opacity() + ( 1-b1a ) *pix.opacity();
			col1.setOpacity ( static_cast<int> ( opacity ) );
			dev->setPixel ( x, (int)yfa, col1 );

			// color first pixel of top line
			if ( ! ( w1==1 && w2==1 ) )
			{
				dev->pixel ( x, (int)yfb, &pix );
				opacity = b1b*c3.opacity() + ( 1-b1b ) *pix.opacity();
				col1.setOpacity ( static_cast<int> ( opacity ) );
				dev->setPixel ( x, (int)yfb, col1 );
			}

			// color second pixel of bottom line
			if ( grada != 0 && grada != 1 ) // if not flat or exact diagonal
			{
				dev->pixel ( x, int ( yfa ) +1, &pix );
				opacity = b2a*c3.opacity() + ( 1-b2a ) *pix.opacity();
				col2.setOpacity ( static_cast<int> ( opacity ) );
				dev->setPixel ( x, int ( yfa ) +1, col2 );
			}

			// color second pixel of top line
			if ( gradb != 0 && gradb != 1 && ! ( w1==1 && w2==1 ) )
			{
				dev->pixel ( x, int ( yfb ) +1, &pix );
				opacity = b2b*c3.opacity() + ( 1-b2b ) *pix.opacity();
				col2.setOpacity ( static_cast<int> ( opacity ) );
				dev->setPixel ( x, int ( yfb ) +1, col2 );
			}

			// fill remaining pixels
			if ( ! ( w1==1 && w2==1 ) )
			{
				if ( yfa<yfb )
					for ( int i=yfa+1; i<=yfb; i++ )
						dev->setPixel ( x,i, c3 );
				else
					for ( int i=yfa+1; i>=yfb; i-- )
						dev->setPixel ( x,i, c3 );
			}

			yfa += grada;
			yfb += gradb;
		}
	}
	else // vertical-ish lines
	{
		if ( y1 < y0 )
		{
			int xt, yt, wt;
			xt = x1a;     x1a = x0a;    x0a = xt;
			yt = y1a;     y1a = y0a;    y0a = yt;
			xt = x1b;     x1b = x0b;    x0b = xt;
			yt = y1b;     y1b = y0b;    y0b = yt;
			xt = x1;      x1 = x0;      x0 = xt;
			yt = y1;      y1 = y0;      y0 = yt;

			KoColor tmp;
			tmp = c1; c1 = c2; c2 = tmp;
			wt = w1;      w1 = w2;      w2 = wt;
		}

		grada = dxa/dya;
		gradb = dxb/dyb;

		ix1 = x0;   iy1 = y0;
		ix2 = x1;   iy2 = y1;

		xfa = x0a + grada;
		xfb = x0b + gradb;

		for ( y = iy1+1; y <= iy2-1; y++ )
		{
			fraca = xfa - int ( xfa );
			b1a = 1 - fraca;
			b2a = fraca;

			fracb = xfb - int ( xfb );
			b1b = 1 - fracb;
			b2b = fracb;

			// color first pixel of left line
			opacity = ( ( y-iy1 ) /dy ) *c2.opacity() + ( 1 - ( y-iy1 ) /dy ) *c1.opacity();
			c3.setOpacity ( static_cast<int> ( opacity ) );

			dev->pixel ( int ( xfa ), y, &pix );
			opacity = b1a*c3.opacity() + ( 1-b1a ) *pix.opacity();
			col1.setOpacity ( static_cast<int> ( opacity ) );
			dev->setPixel ( int ( xfa ), y, col1 );

			// color first pixel of right line
			if ( ! ( w1==1 && w2==1 ) )
			{
				dev->pixel ( int ( xfb ), y, &pix );
				opacity = b1b*c3.opacity() + ( 1-b1b ) *pix.opacity();
				col1.setOpacity ( static_cast<int> ( opacity ) );
				dev->setPixel ( int ( xfb ), y, col1 );
			}

			// color second pixel of left line
			if ( grada != 0 && grada != 1 ) // if not flat or exact diagonal
			{
				dev->pixel ( int ( xfa ) +1, y, &pix );
				opacity = b2a*c3.opacity() + ( 1-b2a ) *pix.opacity();
				col2.setOpacity ( static_cast<int> ( opacity ) );
				dev->setPixel ( int ( xfa ) +1, y, col2 );
			}

			// color second pixel of right line
			if ( gradb != 0 && gradb != 1 && ! ( w1==1 && w2==1 ) )
			{
				dev->pixel ( int ( xfb ) +1, y, &pix );
				opacity = b2b*c3.opacity() + ( 1-b2b ) *pix.opacity();
				col2.setOpacity ( static_cast<int> ( opacity ) );
				dev->setPixel ( int ( xfb ) +1, y, col2 );
			}

			// fill remaining pixels between current xfa,xfb
			if ( ! ( w1==1 && w2==1 ) )
			{
				if ( xfa<xfb )
					for ( int i= ( int ) xfa+1; i<= ( int ) xfb; i++ )
						dev->setPixel ( i,y,c3 );
				else
					for ( int i= ( int ) xfb; i<= ( int ) xfa+1; i++ )
						dev->setPixel ( i,y,c3 );
			}

			xfa += grada;
			xfb += gradb;
		}
	}
}


