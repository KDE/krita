/*
 *  kis_painter.h - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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
#include <qpoint.h>
#include <qrect.h>

class KisDoc;
class KisView;

class KisPainter { //: public QObject {
//	Q_OBJECT

public:
	KisPainter(KisDoc *doc, KisView *view);
	virtual ~KisPainter();

	void resize(int width, int height);
	void clearAll();

	void clearRectangle(int x, int y, int w, int h);
	void clearRectangle(const QRect& rc);
	void clearRectangle(const QPoint& topLeft, const QPoint& bottomRight);

	void drawLine(int x1, int y1, int x2, int y2);
	void drawLine(const QRect& rc);
	void drawLine(const QPoint& topLeft, const QPoint& bottomRight);

	void drawRectangle(int x, int y, int w, int h);
	void drawRectangle(const QRect& rc);    
	void drawRectangle(const QPoint& topLeft, const QPoint& bottomRight);

	void drawEllipse(int x, int y, int w, int h);
	void drawEllipse(const QRect& rc);
	void drawEllipse(const QPoint& topLeft, const QPoint& bottomRight);

	void drawPolygon(const QPointArray& points, const QRect& rect);

	inline void setLineThickness(int t);
	inline void setLineOpacity(int o);
	inline void setFilledEllipse(bool f);
	inline void setFilledRectangle(bool f);
	inline void setFilledPolygon(bool f);
	inline void setGradientFill(bool g);
	inline void setGradientLine(bool g);
	inline void setPatternFill(bool p);
	inline void setPatternLine(bool p);

protected:
	bool toLayer(const QRect& paintRect);

private:
	QImage m_painterImage;
	QPixmap m_painterPixmap;

	KisDoc *m_doc;
	KisView *m_view;

	int  m_lineThickness;
	int  m_lineOpacity;
	bool m_filledEllipse;
	bool m_filledRectangle;
	bool m_filledPolygon;
	bool m_gradientFill;
	bool m_gradientLine;
	bool m_patternFill;
	bool m_patternLine;
};

void KisPainter::setLineThickness(int t)
{ 
	m_lineThickness = t;
}

void KisPainter::setLineOpacity(int o)
{ 
	m_lineOpacity = o;
}

void KisPainter::setFilledEllipse(bool f)   
{ 
	m_filledEllipse = f;
}

void KisPainter::setFilledRectangle(bool f) 
{ 
	m_filledRectangle = f;
}

void KisPainter::setFilledPolygon(bool f)   
{ 
	m_filledPolygon = f;
}

void KisPainter::setGradientFill(bool g)    
{ 
	m_gradientFill = g;
}

void KisPainter::setGradientLine(bool g)    
{ 
	m_gradientLine = g;
}

void KisPainter::setPatternFill(bool p)     
{ 
	m_patternFill = p;
}

void KisPainter::setPatternLine(bool p)     
{ 
	m_patternLine = p;
}

#endif

