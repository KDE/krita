/*
 *  Copyright (c) 2008 Lukas Tvrdy <LukasT.dev@gmail.com>
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
#include "stroke.h"

#include <QApplication>
#include <QLabel>
#include <QImage>
#include <QColor>
#include <QHBoxLayout>
#include <QColormap>

#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;

int inline Stroke::trunc(float value)
{
	return static_cast<int>(value);
}

float inline Stroke::frac(float value)
{
	float tmp = 0;
	return modff(value , &tmp);
}

float inline Stroke::invertFrac(float value){
	float tmp = 0;
	return 1.0f - modff(value , &tmp);
}

int Stroke::MapBrightness(float p, int thickness){
	cout << p << thickness << endl;
	return 0;
}

int Stroke::gsfilter(float val)
{
int pole[] = { 0xc7, 0xc6, 0xc2, 0xbc,
  0xb3, 0xa9, 0x9c, 0x8e,
  0x7f, 0x70, 0x62, 0x54,
  0x46, 0x3a, 0x2f, 0x25,
  0x1c, 0x15, 0x0e, 0x09,
  0x05, 0x03, 0x01, 0x00 };

  float ret;
  if (val < 0) val = -val;

//printf("%lf'n", val);

   //if (val == 0) ret = 1.0;
   if (val < 1/16) ret = pole[0];
   else if (val < 2/16.0) ret = pole[1];
   else if (val < 3/16.0) ret = pole[2];
   else if (val < 4/16.0) ret = pole[3];
   else if (val < 5/16.0) ret = pole[4];
   else if (val < 6/16.0) ret = pole[5];
   else if (val < 7/16.0) ret = pole[6];
   else if (val < 8/16.0) ret = pole[7];
   else if (val < 9/16.0) ret = pole[8];
   else if (val < 10/16.0) ret =pole[9];
   else if (val < 11/16.0) ret =pole[10];
   else if (val < 12/16.0) ret =pole[11];
   else if (val < 13/16.0) ret =pole[12];
   else if (val < 14/16.0) ret =pole[13];
   else if (val < 15/16.0) ret =pole[14];
   else if (val < 16/16.0) ret =pole[15];
   else if (val < 17/16.0) ret =pole[16];
   else if (val < 18/16.0) ret =pole[17];
   else if (val < 19/16.0) ret =pole[18];
   else if (val < 20/16.0) ret =pole[19];
   else if (val < 21/16.0) ret =pole[20];
   else if (val < 22/16.0) ret =pole[21];
   else if (val < 23/16.0) ret =pole[22];
   else ret = pole[23];


int result = (int)(ret);
   cout << "result: " << result << endl;
   return result;

}

void Stroke::drawGSLine(QImage *dev, float x1, float y1, float x2, float y2, float width, const QColor &color){

	Q_UNUSED(width);
	Q_UNUSED(color);
/*
// Width and height of the line
	int xd = (x2 - x1);
	int yd = (y2 - y1);

	// horizontal or vertical lines
	if (fabs(xd) < fabs(yd))
	{
		float tmp;
		tmp=x1;x1=y1;y1=tmp;
		tmp=x2;x2=y2;y2=tmp;
	}

	// line have to be paint from left to right
	if (x1 > x2)
	{
		float tmp;
		tmp=x1;x1=x2;x2=tmp;
		tmp=y1;y1=y2;y2=tmp;
	}

*/

int /*x,*/ y, ix1, ix2,iy1,iy2;

float p,m,c,s;

ix1 = (int)x1;
iy1 = (int)y1;
ix2 = (int)x2;
iy2 = (int)y2;

m = (y2-y1)/(x2-x1);
c = 1/sqrt(m*m+1);
p = 0;
s = (0.5-m)*c;

y = (int)y1;

QColor *black = new QColor(Qt::black);
// main loop

float inc1 = (m-1)*c;
float inc2 = m*c;
for (int x = ix1; x <= ix2 ; x++)
{
	black->setAlpha(gsfilter(p-c));
	dev->setPixel(x, y-1, black->rgba() );
	black->setAlpha(gsfilter(p));
	dev->setPixel(x, y, black->rgba() );
	black->setAlpha(gsfilter(p+c));
	dev->setPixel(x, y+1, black->rgba() );


/*	cout << "p: " << p << endl;
	cout << "p-c: " << p-c << endl;
	cout << "p+c: " << p+c << endl << endl;*/
	if (p >= s)
	{
		y = y + 1;
		p = p+inc1;
	}
	else
	{
		p = p+inc2;
	}

}

}

void Stroke::drawWuLine(QImage *dev, float x1, float y1, float x2, float y2, float width, 
const QColor &color)
{
    Q_UNUSED(width);

// I liked this approach: http://freespace.virgin.net/hugo.elias/graphics/x_wuline.htm

float grad, xd, yd, 
	xgap, xend, yend, yf,
	brightness1, brightness2;

int ix1, ix2, iy1, iy2;

int c1, c2;

const float MaxPixelValue = 255.0f;

// Width and height of the line
	xd = (x2 - x1);
	yd = (y2 - y1);

	// horizontal or vertical lines
	if (fabs(xd) < fabs(yd))
	{
		float tmp;
		tmp=x1;x1=y1;y1=tmp;
		tmp=x2;x2=y2;y2=tmp;
		xd = (x2 - x1);
		yd = (y2 - y1);
	}

	// line have to be paint from left to right
	if (x1 > x2)
	{
		float tmp;
		tmp=x1;x1=x2;x2=tmp;
		tmp=y1;y1=y2;y2=tmp;
		xd = (x2 - x1);
		yd = (y2 - y1);
	}

	grad = yd/xd;

	// nearest X,Y interger coordinates
	xend = round(x1+0.5f);
	yend = y1 + grad * (xend - x1);

	xgap = invertFrac(x1 + 0.5f);

	ix1 = static_cast<int>(xend);
	iy1 = static_cast<int>(yend);

	QColor *black = new QColor(color);

	// calc the intensity of the other end point pixel pair.
	brightness1 = invertFrac(yend) * xgap;
	brightness2 =       frac(yend) * xgap; 

	c1 = (int)(brightness1 * MaxPixelValue);
	c2 = (int)(brightness2 * MaxPixelValue);

	black->setAlpha(c1);
	dev->setPixel(ix1, iy1, black->rgba() );

	black->setAlpha(c2);
	dev->setPixel(ix1, iy1+1, black->rgba() );
	
	// calc first Y-intersection for main loop
	yf = yend+grad;

	xend = trunc(x2+0.5f);
	yend = y2 + grad * (xend - x2);

	xgap = invertFrac(x2-0.5f);

	ix2 = static_cast<int>(xend);
	iy2 = static_cast<int>(yend);

	brightness1 = invertFrac(yend) * xgap; 
	brightness2 =    frac(yend) * xgap; 

	c1 = (int)(brightness1 * MaxPixelValue);
	c2 = (int)(brightness2 * MaxPixelValue);

	black->setAlpha(c1);
    dev->setPixel( ix2,iy2, black->rgba() );

	black->setAlpha(c1);
	dev->setPixel( ix2,iy2+1, black->rgba() );

   // main loop 
   for (int x = ix1+1; x <= ix2-1; x++) {
		brightness1 = invertFrac(yf);
		brightness2 =    frac(yf);
		c1 = (int)(brightness1 * MaxPixelValue);
		c2 = (int)(brightness2 * MaxPixelValue);

		//cout << c1 << " " << c2 << endl;

		black->setAlpha(c1);    
		dev->setPixel(x,int(yf),black->rgba() );
		
 		black->setAlpha(c2);
		dev->setPixel(x,int(yf)+1, black->rgba() );
        yf = yf + grad;
    }

}

void Stroke::gupta_wide_line(QImage *pixmap, int x0, int y0, int x1, int y1, int w1, int w2)
{
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

   if (x1 < x0) // sort so x0<x1
   {  int xt, yt, wt;
      xt = x0;       x0 = x1;       x1 = xt;
      yt = y0;       y0 = y1;       y1 = yt;
      wt = w2;       w2 = w1;       w1 = wt;
   }	

   tp0 = w1/2;
   tn0 = w1/2;
   if (w1%2==0) // even width w1
      tn0--;

   tp1 = w2/2;
   tn1 = w2/2;
   if (w2%2==0) // even width w2
      tn1--;

   int dx = x1 - x0; // run
   int dy = y1 - y0; // rise
   int ady;
   if (dy < 0) ady = -dy;
   else ady = dy;
   int adx;
   if (dx < 0) adx = -dx;
   else adx = dx;

   int x0a, y0a, x1a, y1a, x0b, y0b, x1b, y1b;

   if (adx > ady) // horizontalish
   {  x0a = x0;   y0a = y0 - tn0;
      x0b = x0;   y0b = y0 + tp0;
      x1a = x1;   y1a = y1 - tn1;
      x1b = x1;   y1b = y1 + tp1;
   }
   else
   {  x0a = x0 - tn0;   y0a = y0;
      x0b = x0 + tp0;   y0b = y0;
      x1a = x1 - tn1;   y1a = y1;
      x1b = x1 + tp1;   y1b = y1;
   }

   int dxa, dxb, dya, dyb, adya, adyb;
   dxa = x1a - x0a; // run of a
   dya = y1a - y0a; // rise of a
   dxb = x1b - x0b; // run of b
   dyb = y1b - y0b; // rise of b

   if (dya < 0) adya = -dya;
   else adya = dya;
   if (dyb < 0) adyb = -dyb;
   else adyb = dyb;


   if (dy > 0) // y1>y0 -- line goes right
   {  if (dx >= dy) dir = 1; // more run than rise
      else dir = 2;          // less run than rise
   }
   else  // y1<=y0 -- line goes right
   {  if (dx >= -1*dy) dir = 3;  // more run than rise
      else dir = 4;          // less run than rise
   }

   float perc; int intensity; 
	 QColor *black = new QColor(Qt::black);
   switch (dir) 
   {  case 1:
         Aa = 0;
	 Ba = 1.0/(2.0 * sqrt(dxa *dxa + dya * dya));
	 Ca = 2.0 * dxa * Ba;

         Ab = 0;
	 Bb = 1.0/(2.0 * sqrt(dxb *dxb + dyb * dyb));
	 Cb = 2.0 * dxb * Bb;
    	
         da = 2*dya-dxa;
	 incrEa  = 2*dya;
	 incrNEa = 2*(dya-dxa);
	 ya = y0a;
	 
         db = 2*dyb-dxb;
	 incrEb  = 2*dyb;
	 incrNEb = 2*(dyb-dxb);
	 yb = y0b;
	 
         d = 2*dy-dx;
	 incrE  = 2*dy;
	 incrNE = 2*(dy-dx);
	 x = x0;
	 

	 // line a ------------------------------------------------
	intensity = gsfilter(0);
	black->setAlpha(intensity);
	pixmap->setPixel(x,ya,black->rgba() );
	
	intensity = gsfilter(Ca - Aa*Ba);
        black->setAlpha(intensity);
	pixmap->setPixel(x,ya+1,black->rgba() );

	intensity = gsfilter(Ca + Aa*Ba);
	black->setAlpha(intensity);
	pixmap->setPixel(x,ya+1,black->rgba() );
	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {

	intensity = gsfilter(Ab*Bb);
	black->setAlpha(intensity);
	pixmap->setPixel(x,yb,black->rgba() );

	intensity = gsfilter(Cb - Ab*Bb);
	black->setAlpha(intensity);
	pixmap->setPixel(x,yb+1,black->rgba() );

	intensity = gsfilter(Cb + Ab*Bb);
	black->setAlpha(intensity);
	pixmap->setPixel(x,yb-1,black->rgba() );

	}
	 //--------------------------------------------------------
	 
	 while (x < x1)
	 {   
	     if (da <= 0) 
	     {  Aa = da + dxa;
	        da += incrEa;
	     }
	     else
	     {  Aa = da - dxa;
	        da += incrNEa;
		ya++;
	     }

	     if (db <= 0) 
	     {  Ab = db + dxb;
	        db += incrEb;
	     }
	     else
	     {  Ab = db - dxb;
	        db += incrNEb;
		yb++;
	     }

	     if (d <= 0) 
	     {  d += incrE;
	        x++;
	     }
	     else
	     {  d += incrNE;
		x++;
	     }
             
	     perc = (float)(x-x0)/(float)adx;	     
      
	 // line a ------------------------------------------------
	 intensity = gsfilter(Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya,black->rgba() );

	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya+1,black->rgba() );
	 
	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya-1,black->rgba() );
	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {

	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb,black->rgba() );

	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb+1,black->rgba() );

	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb-1,black->rgba() );

         }
	 //--------------------------------------------------------
		black->setAlpha(255); // c3
            // fill remaining pixels between current xfa,xfb
	    if (!(w1==1 && w2==1))
	    {  if (ya<yb)
                 for (int i=ya; i<=yb; i++)
			pixmap->setPixel(x,i,black->rgba() );
	       else
                 for (int i=yb; i<=ya; i++)
             		pixmap->setPixel(x,i,black->rgba() );
            }

	 }
	 
	 break;

      case 2: 
         Aa = 0;
	 Ba = 1.0/(2.0 * sqrt(dxa *dxa + dya * dya));
	 Ca = 2.0 * dxa * Ba;

         Ab = 0;
	 Bb = 1.0/(2.0 * sqrt(dxb *dxb + dyb * dyb));
	 Cb = 2.0 * dxb * Bb;
    	
         da = dya-2*dxa;
	 incrNa  = -2*dxa;
	 incrNEa = 2*(dya-dxa);
	 xa = x0a;
	 
         db = dyb-2*dxb;
	 incrNb  = -2*dxb;
	 incrNEb = 2*(dyb-dxb);
	 xb = x0b;

         d = dy-2*dx;
	 incrN  = -2*dx;
	 incrNE = 2*(dy-dx);
	 y = y0;
	 
	 // line a ------------------------------------------------
	 intensity = gsfilter(0);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa,y,black->rgba() );
			
	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa+1,y,black->rgba() );
	 
	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa-1,y,black->rgba() );
	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {

	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb,y,black->rgba() );
	 
	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb+1,y,black->rgba() );
	 
	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb-1,y,black->rgba() );
         }
	 //--------------------------------------------------------

	 while (y < y1)
	 {  
	    if (da <= 0)
	    {  Aa = da + dya;
	        da += incrNEa;
	       xa++;
	    }
	    else 
	     {  Aa = da - dya;
	      da += incrNa;
	     }
	    
	    if (db <= 0)
	    {  Ab = db + dyb;
	        db += incrNEb;
	       xb++;
	    }
	    else 
	     {  Ab = db - dyb;
	      db += incrNb;
	     }

	    if (d <= 0)
	    {  d += incrNE;
	       y++;
	    }
	    else 
	    {  d += incrN;
	       y++;
	    }

             perc = (float)(y-y0)/(float)ady;	     
	     /*c3.r = perc*c2.r + (1 - perc)*c1.r;
	     c3.g = perc*c2.g + (1 - perc)*c1.g;
	     c3.b = perc*c2.b + (1 - perc)*c1.b;*/
      	 
	 
	 // line a ------------------------------------------------
	 intensity = gsfilter(Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa,y,black->rgba() );
	 
	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa+1,y,black->rgba() );

	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa-1,y,black->rgba() );
	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {
	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb,y,black->rgba() );
	 
	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb+1,y,black->rgba() );
	 
	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb-1,y,black->rgba() );
         }
	 //--------------------------------------------------------

	black->setAlpha(255);
            // fill remaining pixels between current xfa,xfb
	    if (!(w1==1 && w2==1))
	    {  if (xa<xb)
                 for (int i=xa; i<=xb; i++)
		    pixmap->setPixel(i,y,black->rgba() );
                    
	       else
                 for (int i=xb; i<=xa; i++)
		    pixmap->setPixel(i,y,black->rgba() );
            }
	 }
	 
	 break;

      case 3: 
         Aa = 0;
	 Ba = 1.0/(2.0 * sqrt(dxa *dxa + dya * dya));
	 Ca = 2.0 * dxa * Ba;

         Ab = 0;
	 Bb = 1.0/(2.0 * sqrt(dxb *dxb + dyb * dyb));
	 Cb = 2.0 * dxb * Bb;
    	
         da = 2*dya+dxa;	        
	 incrEa  = 2*dya;
	 incrSEa = 2*(dya+dxa);
	 ya = y0a;
	 
         db = 2*dyb+dxb;	        
	 incrEb  = 2*dyb;
	 incrSEb = 2*(dyb+dxb);
	 yb = y0b;
	 
         d = 2*dy+dx;	        
	 incrE  = 2*dy;
	 incrSE = 2*(dy+dx);
	 x = x0;
		
	 // line a ------------------------------------------------
	 intensity = gsfilter(0);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya,black->rgba() );
	 		    
	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya+1,black->rgba() );
	 
	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya-1,black->rgba() );
	 
	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {
	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb,black->rgba() );

	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb+1,black->rgba() );

	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb-1,black->rgba() );
         }
	 //--------------------------------------------------------
	 
	 while (x < x1)
	 {  
	    if (da <= 0) 
	    {  Aa = da + dxa;
	       da += incrSEa;
	       ya--;
	    }
	    else
	    {  Aa = da - dxa;
	      da += incrEa;
	    }
	      
	    if (db <= 0) 
	    {  Ab = db + dxb;
	       db += incrSEb;
	       yb--;
	    }
	    else
	     {  Ab = db - dxb;
	      db += incrEb;
	     }

	    if (d <= 0) 
	    {  d += incrSE;
	       x++;
	    }
	    else
	    {  d += incrE;
	       x++;
	    }
	    
             perc = (float)(x-x0)/(float)adx;	     
      	 
	 // line a ------------------------------------------------
	 intensity = gsfilter(Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya,black->rgba() );
	 
	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya+1,black->rgba() );

	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,ya-1,black->rgba() );

	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {
	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb,black->rgba() );

	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb+1,black->rgba() );


	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(x,yb-1,black->rgba() );
         }
	 //--------------------------------------------------------
		black->setAlpha(255);
            // fill remaining pixels between current xfa,xfb
	    if (!(w1==1 && w2==1))
	    {  if (ya<yb)
                 for (int i=ya; i<=yb; i++)
		    pixmap->setPixel(x,i,black->rgba() );
                    
	       else
                 for (int i=yb; i<=ya; i++)
                    pixmap->setPixel(x,i,black->rgba() );
            }
 	 }
	 
	 break;

    case 4: 
         Aa = 0;
	 Ba = 1.0/(2.0 * sqrt(dxa *dxa + dya * dya));
	 Ca = 2.0 * dxa * Ba;

         Ab = 0;
	 Bb = 1.0/(2.0 * sqrt(dxb *dxb + dyb * dyb));
	 Cb = 2.0 * dxb * Bb;
    	
         da = dya+2*dxa;
	 incrSEa = 2*(dya+dxa);
	 incrSa  = 2*dxa;
	 xa = x0a;
	 
         db = dyb+2*dxb;
	 incrSEb = 2*(dyb+dxb);
	 incrSb  = 2*dxb;
	 xb = x0b;
		
         d = dy+2*dx;
	 incrSE = 2*(dy+dx);
	 incrS  = 2*dx;
	 y = y0;
		
	 
	 // line a ------------------------------------------------
	 intensity = gsfilter(0);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa,y,black->rgba() );

	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa+1,y,black->rgba() );

	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa-1,y,black->rgba() );

	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {
	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb,y,black->rgba() );

	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb+1,y,black->rgba() );
	 

	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb-1,y,black->rgba() );
	 
         }
	 //--------------------------------------------------------

	 while (y > y1) 
	 {   
	    if (da <= 0)
	     {  Aa = da + dya;
	      da += incrSa;
	     }
	    else
	    {  Aa = da - dya;
	        da += incrSEa;
	       xa++;
	    }
	    
	    if (db <= 0)
	     {  Ab = db + dyb;
	      db += incrSb;
	     }
	    else
	    {  Ab = db - dyb;
	        db += incrSEb;
	       xb++;
	    }
	    
	    if (d <= 0)
	    {  d += incrS;
	       y--;
	    }
	    else
	    {  d += incrSE;
	       y--;
	    }
		   
             perc = (float)(y0-y)/(float)ady;	     
	 
	 // line a ------------------------------------------------
	 intensity = gsfilter(Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa,y,black->rgba() );

	 intensity = gsfilter(Ca - Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa+1,y,black->rgba() );


	 intensity = gsfilter(Ca + Aa*Ba);
         black->setAlpha(intensity);
	 pixmap->setPixel(xa-1,y,black->rgba() );
	 
	 
	 //--------------------------------------------------------
	 // line b ------------------------------------------------
         //if (!(w1==1 && w2==1))
	 {
	 intensity = gsfilter(Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb,y,black->rgba() );

	 intensity = gsfilter(Cb - Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb+1,y,black->rgba() );

	 intensity = gsfilter(Cb + Ab*Bb);
         black->setAlpha(intensity);
	 pixmap->setPixel(xb-1,y,black->rgba() );
	}
	 //--------------------------------------------------------
	    black->setAlpha(255);
            // fill remaining pixels between current xfa,xfb
	    if (!(w1==1 && w2==1))
	    {  if (xa<xb)
                 for (int i=xa; i<=xb; i++)
                    pixmap->setPixel(i,y,black->rgba() );
	       else
                 for (int i=xb; i<=xa; i++)
                    pixmap->setPixel(i,y,black->rgba() );
            }
	 }
	 
	 break;	
   }
   
}




int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QWidget *window = new QWidget;
	int w=640,h=480;
	QImage *image = new QImage(w,h,QImage::Format_ARGB32);

	int centerX = w/2;
	int centerY = h/2;

	uint c = 0xFFFF0000;
	image->setPixel(centerX, centerY, c);

	Stroke stroke;
	
	//stroke.drawWuLine(image,0,0,320,240,20, Qt::black);
	stroke.gupta_wide_line(image,320,240,0,0,20,20);
	//stroke.drawWuLine(image,0,5,320,240,20, Qt::black);
	

int WU = 0;
int GS = 0;
//int GS1 = 0;

if (WU){
	stroke.drawWuLine(image,0,0,centerX,centerY,20,Qt::red);
	stroke.drawWuLine(image,640,0,centerX,centerY,20,Qt::green);
	
	stroke.drawWuLine(image,0,480,centerX,centerY,20,Qt::blue);
	stroke.drawWuLine(image,640,480,centerX,centerY,20,Qt::magenta);

	stroke.drawWuLine(image,0,180,centerX,centerY,20,Qt::black);
	stroke.drawWuLine(image,40,480,centerX,centerY,20,Qt::black);
}
if (GS)
{
	stroke.drawGSLine(image,0,0,centerX,centerY,20,Qt::red);
	stroke.drawGSLine(image,640,0,centerX,centerY,20,Qt::green);
	
	stroke.drawGSLine(image,0,480,centerX,centerY,20,Qt::blue);
	stroke.drawGSLine(image,640,480,centerX,centerY,20,Qt::magenta);

	stroke.drawGSLine(image,0,180,centerX,centerY,20,Qt::black);
	stroke.drawGSLine(image,40,480,centerX,centerY,20,Qt::black);
}

	


/*	for (int i=0;i<10;i++){
		int x = (random() % h);
		int y = (random() % w);
		drawWuLine(image,x,y,centerX,centerY,20,Qt::black);
		cout << x << ":" <<y << endl;
		cout << "[*]" << centerX << ":" <<centerY << endl;
	}*/

	QLabel *label = new QLabel();	

	label->setPixmap(QPixmap::fromImage(*image));

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(label);
	window->setLayout(layout);
	
	window->show();

	return app.exec();
}
