/* This file is part Krita

  Copyright (c) 2004 Sven Langkamp <longamp@reallygood.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KIS_COLORWHEEL_H
#define KIS_COLORWHEEL_H

#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>

#include <kselect.h> 
#include <koColor.h> 
 
class KisColorWheel : public KXYSelector
{
  Q_OBJECT

public:
	KisColorWheel( QWidget *parent=0, const char *name=0 );

signals:
	void valueChanged(const KoColor& c);

protected:
	virtual void drawWheel( QPixmap *pixmap );
	virtual void resizeEvent( QResizeEvent * );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void drawContents( QPainter *painter );

private:
	QPixmap m_pixmap;
	KoColor m_color;
};

#endif
