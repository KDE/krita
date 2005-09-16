/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "qpixmap.h"
#include "qlayout.h"
#include "qpainter.h"

#include "knuminput.h"

#include "koDocument.h"
 
#include "wdgbirdeye.h"
#include "kobirdeyepanel.h"

KoZoomListener::KoZoomListener()
{
}

KoZoomListener::~KoZoomListener()
{
}

KoThumbnailProvider::KoThumbnailProvider()
{
}

KoThumbnailProvider::~KoThumbnailProvider()
{
}

KoBirdEyePanel::KoBirdEyePanel( KoZoomListener * zoomListener, 
                                KoThumbnailProvider * thumbnailProvider,
                                QWidget * parent,
                                const char * name,
                                WFlags f)
    : QWidget(parent, name, f)
    , m_zoomListener(zoomListener)
{
    QHBoxLayout * l = new QHBoxLayout(this);
    m_page = new WdgBirdEye(this);
    m_page->zoom->setRange(zoomListener->getMinZoom(), zoomListener->getMaxZoom(), 10, true);
    m_page->zoom->setValue(100);
    
    QRect r = thumbnailProvider->pixelSize();
    
    QPixmap pix = thumbnailProvider->image(r);
    QPainter p(m_page->view);
    //p.scale(100 / r.width(), 100 / r.height());
    p.drawPixmap(0, 0, pix, 0, 0);
    p.end();
    
    l->addWidget(m_page);


}

KoBirdEyePanel::~KoBirdEyePanel()
{
}

void KoBirdEyePanel::slotCanvasZoomChanged(int zoom)
{
}

void KoBirdEyePanel::slotUpdate(const QRect & r, const QImage & img, const QRect & docrect)
{
}

#include "kobirdeyepanel.moc"
