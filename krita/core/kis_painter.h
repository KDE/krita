/*
 *  kis_gradient.h - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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

#ifndef __kis_painter_h__
#define __kis_painter_h__

#include <qobject.h>
#include <qimage.h>
#include <qpixmap.h>
#include "kis_doc.h"

class KisDoc;

class KisPainter : public QObject 
{
  Q_OBJECT

public:
  	KisPainter(KisDoc *doc, KisView *view);
  	~KisPainter();

    void resize(int width, int height);
    void clearAll();
    void clearRectangle(QRect & rect);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRectangle(int x, int y, int w, int h);
    void drawRectangle(QRect & rectint);    
    void drawEllipse(int x, int y, int w, int h);
    void drawEllipse(QRect & rect);
    void drawPolygon(QPointArray & points, QRect & rect);

    void  setLineThickness(int t)    { lineThickness = t;}
    void  setLineOpacity(int o)      { lineOpacity = o;}
    void  setFilledEllipse(bool f)   { filledEllipse = f;}
    void  setFilledRectangle(bool f) { filledRectangle = f;}
    void  setFilledPolygon(bool f)   { filledPolygon = f;}
    void  setGradientFill(bool g)    { gradientFill = g;}
    void  setGradientLine(bool g)    { gradientLine = g;}
    void  setPatternFill(bool p)     { patternFill = p;}
    void  setPatternLine(bool p)     { patternLine = p;}
      
protected:
    bool toLayer(QRect paintRect);

private:
    void swap(int *first, int *second);
  	QImage painterImage;
  	QPixmap painterPixmap;

  	KisDoc *pDoc;
    KisView *pView;
    
    int  lineThickness;
    int  lineOpacity;
    bool filledEllipse;
    bool filledRectangle;
    bool filledPolygon;
    bool gradientFill;
    bool gradientLine;
    bool patternFill;
    bool patternLine;
};

#endif

