/*
 *  kis_canvas.cc - part of KImageShop
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
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "kis_canvas.h"
#include "kis_cursor.h"

KisCanvas::KisCanvas( QWidget* parent, const char* name ) 
    : QWidget( parent, name )
{
    setBackgroundMode( QWidget::NoBackground );
    setMouseTracking( true );
}

void KisCanvas::showScrollBars()
{
    int w = width();
    int h = height();

    resize(w - 1, h - 1);
    resize(w, h);
}

void KisCanvas::paintEvent( QPaintEvent* e )
{
    emit gotPaintEvent(e);
}

void KisCanvas::mousePressEvent ( QMouseEvent *e )
{
    emit mousePressed(e);
}

void KisCanvas::mouseReleaseEvent ( QMouseEvent *e )
{
    emit mouseReleased(e);
}

void KisCanvas::mouseMoveEvent ( QMouseEvent *e )
{
    emit mouseMoved(e);
}

void KisCanvas::enterEvent( QEvent*e )
{
    emit gotEnterEvent(e);
}

void KisCanvas::leaveEvent( QEvent*e )
{
    emit gotLeaveEvent(e);
}

void KisCanvas::wheelEvent( QWheelEvent *e )
{
    emit mouseWheelEvent( e );
}

#include "kis_canvas.moc"
