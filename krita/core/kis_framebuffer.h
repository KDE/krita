/*
 *  kis_framebuffer.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kis_framebuffer_h__
#define __kis_framebuffer_h__

#include <qobject.h>
#include <qimage.h>
#include <qcolor.h>

#include "kis_gradient.h"
#include "kis_doc.h"
 
class KisSelection;
class KisPattern;
class KisLayer;

class KisFrameBuffer : public QObject 
{
    Q_OBJECT

public:

    KisFrameBuffer(KisDoc *doc);
    ~KisFrameBuffer();

    void setNull();
    bool eraseCurrentLayer();
    
    QImage & getImage() { return srcImage; }
    void setImage(QImage & img);
    
    QRect  & getRect() { return destRect; } 
    void setRect(QRect & rect);

    bool changeColors(uint oldColor, uint newColor, 
        QRect & r, KisSelection *s = 0L); 

    bool scaleSmooth(QRect & srcR, int newWidth, int newHeight);
    bool scaleRough(QRect & srcR, int newWidth, int newHeight);    

    bool mirror(QRect & r);
    bool flip(QRect & r);
    bool rotate90(QRect & r);
    bool rotate180(QRect & r);
    bool rotate270(QRect & r);
    
    bool QImageToLayer(QImage *qimg, QRect & src, QRect & dest);
    bool layerToQImage(QImage *qimg, QRect & src, QRect & dest);
    
    void setPattern(KisPattern *pattern);
    void setPatternToPixel(KisLayer *lay, int x, int y, uint value);
    
    void setGradientPaint(bool _gradientPaint, 
        KisColor startColor, KisColor endColor);
    void setGradientEffect(KImageEffect::GradientType effect)
        { mGradient.setEffect(effect); }     
    void setGradientToPixel(KisLayer *lay, int x, int y);
    KisGradient & gradient( ) { return mGradient; }   
    
protected:

    QImage srcImage;
    QRect  destRect;
    QRect  srcRect;
    
    void addScratchLayer(int width, int height);
    void removeScratchLayer();
          
private:

    KisDoc *pDoc;
    KisLayer        *pScratchLayer;
    KisSelection    *pSelection;
    KisPattern      *pPenPattern;

    KisGradient mGradient;
    
    bool mPatternPaint;
    bool mGradientPaint;
};

#endif

