/*
 *  kis_brushwidget.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
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

#include <qpainter.h>

#include "kis_brush.h"
#include "kis_brushwidget.h"

#define ITEMSIZE 34

KisBrushWidget::KisBrushWidget( QWidget* parent, const char* name ) 
    : QFrame( parent, name )
{
    setBackgroundColor( white );
    setFrameStyle( Panel | Sunken );
}

void KisBrushWidget::slotSetBrush( KisBrush& b)
{
    m_pBrush = &b;
    repaint();
}

void KisBrushWidget::drawContents ( QPainter *p )
{
    if (!m_pBrush || !m_pBrush->isValid())
        return;
  
    int x = 0;
    int y = 0;

    IconItem *item = (IconItem *)m_pBrush;
    
    if ((m_pBrush->width() < ITEMSIZE && m_pBrush->height() < ITEMSIZE)
    || (!item->hasValidThumb()))
    {
        if (m_pBrush->width() < ITEMSIZE)
            x = (ITEMSIZE - m_pBrush->width()) / 2;

        if (m_pBrush->height() < ITEMSIZE)
            y = (ITEMSIZE - m_pBrush->height()) / 2;

        p->drawPixmap(x, y, m_pBrush->pixmap()); 
    }    
    else if (item->hasValidThumb())
    {    
        if (m_pBrush->thumbPixmap().width() < ITEMSIZE)
            x = (ITEMSIZE - m_pBrush->thumbPixmap().width()) / 2;

        if (m_pBrush->thumbPixmap().height() < ITEMSIZE)
            y = (ITEMSIZE - m_pBrush->thumbPixmap().height()) / 2;

        p->drawPixmap(x, y, m_pBrush->thumbPixmap()); 
    }    
    
}

void KisBrushWidget::mousePressEvent ( QMouseEvent * )
{
    emit clicked();
}

#include "kis_brushwidget.moc"
