/***************************************************************************
 *   Copyright (C) 2005 Eyal Lotem <eyal.lotem@gmail.com>                  *
 *   Copyright (C) 2005 Alexandre Oliveira <aleprj@gmail.com>              *
 *   Copyright (C) 2006 Cyrille Berger <cberger@cberger.net>               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/
#include "imageviewer.h"

#include <QLabel>
#include <QPainter>
#include <QImage>
#include <QCursor>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>

#include <kapplication.h>
#include <kdebug.h>
#include <kis_cursor.h>

ImageViewer::ImageViewer(QWidget *widget, const char * name)
    : Q3ScrollView(widget, name)
    , m_isDragging(false)
    , m_image(QPixmap())
{
    m_label = new QLabel( viewport());
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setCursor(KisCursor::handCursor());
    addChild(m_label);
}

void ImageViewer::setImage(QImage & image)
{
    m_image = QPixmap::fromImage(image);
    m_label->setPixmap(m_image);
    resizeContents( m_image.width(), m_image.height() );
    repaintContents(false);
}

void ImageViewer::contentsMousePressEvent(QMouseEvent *event)
{
    if(Qt::LeftButton == event->button()) {
        setCursor(KisCursor::closedHandCursor());
        m_currentPos = event->globalPos();
        m_isDragging = true;
    }
}

void ImageViewer::contentsMouseReleaseEvent(QMouseEvent *event)
{
    if(Qt::LeftButton == event->button()) {
        setCursor(KisCursor::handCursor());
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

#include "imageviewer.moc"
