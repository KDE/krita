/*
 *  kis_patternwidget.cc - part of Krayon
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

#include "kis_pattern.h"
#include "kis_patternwidget.h"

#define ITEMSIZE 34

KisPatternWidget::KisPatternWidget( QWidget* parent, const char* name ) 
    : QFrame( parent, name )
{
    setBackgroundColor( white );
    setFrameStyle( Panel | Sunken );
    
    if (name) 
        m_pPattern = new KisPattern(name);
    else    
        m_pPattern = new KisPattern(0);    
}

void KisPatternWidget::slotSetPattern( KisPattern& b)
{
    m_pPattern = &b;
    repaint();
}

void KisPatternWidget::drawContents ( QPainter *p )
{
    if (!m_pPattern || !m_pPattern->isValid())
        return;
  
    int x = 0;
    int y = 0;
    
    IconItem *item = (IconItem *)m_pPattern;
    
    if ((m_pPattern->width() < ITEMSIZE && m_pPattern->height() < ITEMSIZE)
    || (!item->hasValidThumb()))
    {
        if (m_pPattern->width() < ITEMSIZE)
            x = (ITEMSIZE - m_pPattern->width()) / 2;

        if (m_pPattern->height() < ITEMSIZE)
            y = (ITEMSIZE - m_pPattern->height()) / 2;

        p->drawPixmap(x, y, m_pPattern->pixmap()); 
    }    
    else if (item->hasValidThumb())
    {    
        if (m_pPattern->thumbPixmap().width() < ITEMSIZE)
            x = (ITEMSIZE - m_pPattern->thumbPixmap().width()) / 2;

        if (m_pPattern->thumbPixmap().height() < ITEMSIZE)
            y = (ITEMSIZE - m_pPattern->thumbPixmap().height()) / 2;

        p->drawPixmap(x, y, m_pPattern->thumbPixmap()); 
    }    
}

void KisPatternWidget::mousePressEvent ( QMouseEvent * )
{
  emit clicked();
}

#include "kis_patternwidget.moc"
