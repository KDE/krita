/*
 *  kis_gradient.h - part of Krayon
 *
 *  Copyright (c) 2001 John Califf <jcaliff@compuzone.net>
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

#ifndef __kis_gradient_h__
#define __kis_gradient_h__

#include <qimage.h>
#include <qcolor.h>

#include <kimageeffect.h>

#include <koColor.h>

class KisGradient 
{
 public:

	KisGradient();
	~KisGradient();
    
	void setNull();
    
	void setEffect(KImageEffect::GradientType _effect)
		{ mEffect = _effect; }
        
	/* using kde's native kimageeffect gradients for 2 color
	   gradients may be preferred - how to handle gimp gradients
	   is another matter and requires a translation method - this
	   will be done with plugins for a later release - not a high
	   priority at all for the Krayon release 1.0 and Gimp gradients will
	   be redone for Gimp 2.0 anyway */
    
	void mapKdeGradient(QRect gradR, 
			    KoColor startColor, KoColor endColor);
        	
	/*  these probably won't be used without copying kimageffect code
	    and modifying for use with krayon - use what's above.  Eventually
	    all kimageeffect gradients will need to be relplaced with native
	    gradients for better flexibility and 32 bit (or 64 bit) rendering */
    
	void mapVertGradient(QRect gradR, 
			     KoColor startColor, KoColor endColor);
	void mapHorGradient(QRect gradR, 
			    KoColor startColor, KoColor endColor);
     
	const int width()  { return mGradientWidth; }
	const int height() { return mGradientHeight; }
    
	uint arrayPixelValue(int x, int y) 
		{ return gradArray[y * mGradientWidth + x]; }

	uint imagePixelValue(int x, int y)
		{ return *((uint *)gradImage.scanLine(y)  + x); }    
    
	KImageEffect::GradientType gradientType() { return mEffect; }
        
	QImage gradient(const QSize &size, const QColor &ca,
			const QColor &cb, KImageEffect::GradientType eff, int ncols);
        
	QImage& dither(QImage &img, const QColor *palette, int size);
    
	int nearestColor( int r, int g, int b, const QColor *palette, int size );
    

 private:
    
	QMemArray <uint> gradArray;
	QImage gradImage;
        
	int mGradientWidth;
	int mGradientHeight;

	KImageEffect::GradientType mEffect;
    
};

#endif

