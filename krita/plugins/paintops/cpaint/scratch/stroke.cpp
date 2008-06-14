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

void Stroke::drawWuLine(QImage *dev, float x1, float y1, float x2, float y2, float width, 
const QColor &color)
{
    Q_UNUSED(width);
    Q_UNUSED(color);

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

	stroke.drawWuLine(image,0,0,centerX,centerY,20,Qt::red);
	stroke.drawWuLine(image,640,0,centerX,centerY,20,Qt::green);
	
	stroke.drawWuLine(image,0,480,centerX,centerY,20,Qt::blue);
	stroke.drawWuLine(image,640,480,centerX,centerY,20,Qt::magenta);

	stroke.drawWuLine(image,0,180,centerX,centerY,20,Qt::black);
	stroke.drawWuLine(image,40,480,centerX,centerY,20,Qt::black);


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
