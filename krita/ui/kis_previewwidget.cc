/*
 *  kis_previewwidget.cc - part of Krayon
 *
 *  Copyright (c) 2001 John Califf  <jwcaliff@compuzone.net>
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

#include "kis_layer.h"
#include "kis_previewwidget.h"

#define ITEMSIZE 34

KisPreviewWidget::KisPreviewWidget( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    setBackgroundColor( white );
    setFrameStyle( Panel | Sunken );
    // pix.resize(ITEMSIZE, ITEMSIZE);
}

void KisPreviewWidget::slotSetPreview(KisLayer * /*lay*/)
{
    // scale passed in layer here
    repaint();
}

void KisPreviewWidget::drawContents ( QPainter */*p*/ )
{
    //int x = 0;
    //int y = 0;

    // p->drawPixmap(x, y, pix);
}

void KisPreviewWidget::mousePressEvent ( QMouseEvent * )
{
    emit clicked();
    // take snapshot of current layer
}

#include "kis_previewwidget.moc"
