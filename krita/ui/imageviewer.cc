/***************************************************************************
 *   Copyright (C) 2005 Eyal Lotem <eyal.lotem@gmail.com>                  *
 *   Copyright (C) 2005 Alexandre Oliveira <aleprj@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/
#include "imageviewer.h"

#include <kapplication.h>
#include <kdebug.h>

#include <qpainter.h>
#include <qimage.h>

ImageViewer::ImageViewer(QWidget *widget, const char * name)
    : QScrollView(widget, name)
    , m_isDragging(false)
    , m_image(QImage())
{
    resizeContents(0, 0);
}

void ImageViewer::setImage(QImage & image)
{
    m_image = image;
    resizeContents( m_image.width(), m_image.height() );
    repaintContents(false);
}    

void ImageViewer::drawContents( QPainter * p, int clipx, int clipy, int clipw, int cliph )
{
    p->drawImage(QPoint(clipx, clipy),
                 m_image,
                 QRect(clipx, clipy, clipw, cliph));
}

void ImageViewer::contentsMousePressEvent(QMouseEvent *event)
{
    if(LeftButton == event->button()) {
        m_currentPos = event->globalPos();
        m_isDragging = true;
    }
}

void ImageViewer::contentsMouseReleaseEvent(QMouseEvent *event)
{
    if(LeftButton == event->button()) {
        m_currentPos = event->globalPos();
        m_isDragging = false;
    }
}

void ImageViewer::contentsMouseMoveEvent(QMouseEvent *event)
{
    if(m_isDragging) {
        QPoint delta = m_currentPos - event->globalPos();
        scrollBy(delta.x(), delta.y());
        m_currentPos = event->globalPos();
    }
}

QSize ImageViewer::maximalSize()
{
    return m_image.size().boundedTo( KApplication::desktop()->size() ) + size() - viewport()->size();
}

#include "imageviewer.moc"
