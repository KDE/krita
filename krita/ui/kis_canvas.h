/*
 *  kis_canvas.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
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

#ifndef __kis_canvas_h__
#define __kis_canvas_h__

#include <qwidget.h>

class KisCanvas : public QWidget
{
  Q_OBJECT

 public:
    KisCanvas( QWidget* parent = 0, const char* name = 0 );
    void showScrollBars();
    
 signals:
    void mousePressed( QMouseEvent * );
    void mouseMoved( QMouseEvent * );
    void mouseReleased( QMouseEvent * );
    void gotPaintEvent( QPaintEvent* );
    void gotEnterEvent( QEvent* );
    void gotLeaveEvent( QEvent* );  
    void mouseWheelEvent( QWheelEvent * );
  
 protected:
    virtual void paintEvent( QPaintEvent* );
    virtual void mousePressEvent ( QMouseEvent * );
    virtual void mouseReleaseEvent ( QMouseEvent * );
    virtual void mouseMoveEvent ( QMouseEvent * );
    virtual void enterEvent( QEvent* );
    virtual void leaveEvent( QEvent* );
    virtual void wheelEvent( QWheelEvent * );
};

#endif
