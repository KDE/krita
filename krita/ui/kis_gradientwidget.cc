/*
 *  kis_gradientwidget.cc - part of Krayon
 *
 *  Copyright (c) 2001 John Califf  <jcaliff@compuzone.net>
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
#include <qimage.h>
#include "kis_gradient.h"
#include "kis_gradientwidget.h"

#define ITEMSIZE 34

KisGradientWidget::KisGradientWidget( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    setBackgroundColor( white );
    setFrameStyle( Panel | Sunken );
}

void KisGradientWidget::slotSetGradient( const KisGradient& g)
{
    m_pGradient = &g;
    repaint();
}

void KisGradientWidget::drawContents ( QPainter */*p*/ )
{
    if (!m_pGradient)
        return;

    //int x = 0;
    //int y = 0;

    // IconItem *item = (IconItem *)m_pGradient;

#if 0
    if (item->hasValidThumb())
    {
        if (m_pBrush->thumbPixmap().width() < ITEMSIZE)
            x = (ITEMSIZE - m_pBrush->thumbPixmap().width()) / 2;

        if (m_pBrush->thumbPixmap().height() < ITEMSIZE)
            y = (ITEMSIZE - m_pBrush->thumbPixmap().height()) / 2;

        p->drawPixmap(x, y, m_pBrush->thumbPixmap());
    }
#endif
}

void KisGradientWidget::mousePressEvent ( QMouseEvent * )
{
    emit clicked();
}

#include "kis_gradientwidget.moc"
