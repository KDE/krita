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
#include <qpixmap.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qframe.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <knuminput.h>
#include <klocale.h>

#include <koDocument.h>
 
#include "wdgbirdeye.h"
#include "kobirdeyepanel.h"

KoCanvasAdapter::KoCanvasAdapter() {}
KoCanvasAdapter::~KoCanvasAdapter() {}

KoZoomAdapter::KoZoomAdapter() {}
KoZoomAdapter::~KoZoomAdapter() {}

KoThumbnailAdapter::KoThumbnailAdapter() {}
KoThumbnailAdapter::~KoThumbnailAdapter() {}

KoBirdEyePanel::KoBirdEyePanel( KoZoomAdapter * zoomListener, 
                                KoThumbnailAdapter * thumbnailProvider,
                                KoCanvasAdapter * canvas,
                                QWidget * parent,
                                const char * name,
                                WFlags f)
    : QWidget(parent, name, f)
    , m_zoomListener(zoomListener)
    , m_thumbnailProvider(thumbnailProvider)
    , m_canvas(canvas)
{
    QHBoxLayout * l = new QHBoxLayout(this);
    m_page = new WdgBirdEye(this);
    m_page->zoom->setRange((int) (100 * zoomListener->getMinZoom()), (int) (100 * zoomListener->getMaxZoom()), 10, true);
    m_page->zoom->setValue(100);
    m_page->zoom->setSuffix("%");
    
    m_page->toolbar->setIconSize(16);
    m_page->view->installEventFilter(this);
    
    m_zoomIn = new KAction( i18n("Zoom In"), "birdeye_zoom_plus", 0, this, SLOT(zoomPlus()), this, "zoomIn" );
    m_zoomOut = new KAction( i18n("Zoom Out"), "birdeye_zoom_minus", 0, this, SLOT(zoomMinus()), this, "zoomOut" );

    

    
    l->addWidget(m_page);

    connect(m_page->zoom, SIGNAL(valueChanged(int)), SLOT(zoomValueChanged(int)));
}

void KoBirdEyePanel::zoomValueChanged(int zoom)
{
    KoPoint center;
    center = m_canvas->visibleArea().center();

    m_zoomListener->zoomTo(center.x(), center.y(), zoom / 100.0);
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

void KoBirdEyePanel::zoomMinus()
{
}

void KoBirdEyePanel::zoomPlus()
{
}

void KoBirdEyePanel::updateView()
{
    updateVisibleArea();
}


void KoBirdEyePanel::updateVisibleArea()
{
    KoRect visibleRect = m_canvas->visibleArea();
    QRect canvasSize = m_canvas->size();
    QSize viewSize = m_page->view->size();
    
    int px0 = (viewSize.width() - canvasSize.width()) / 2;
    int py0 = (viewSize.height() - canvasSize.width()) / 2;

    int x = visibleRect.x() + px0;
    int y = visibleRect.y() + py0;
    int w = visibleRect.width();
    int h = visibleRect.height();
    
    QRect r = m_thumbnailProvider->pixelSize();
    QImage img = m_thumbnailProvider->image(r);
    img = img.smoothScale(m_page->view->width(), m_page->view->height(), QImage::ScaleMin);
    
    QPainter painter(m_page->view);
    
    painter.fillRect(0, 0, m_page->view->width(), m_page->view->height(), KGlobalSettings::baseColor());
    
    int imgx = 0;
    int imgy = 0;
    if (img.width() < 150) imgx = (150 - img.width()) / 2;
    if (img.height() < 150) imgy = (150 - img.height()) / 2;

    painter.drawImage(imgx, imgy, img, 0, 0, img.width(), img.height());

    painter.setPen(red);
    painter.drawRect(x, y, w, h);
    painter.setPen(red.light());
    painter.drawRect(x-1, y-1, w+2, h+2);
    painter.end();

    m_visibleArea.setRect(x, y, w, h);
}


bool KoBirdEyePanel::eventFilter(QObject* o, QEvent* ev)
{
    if (o == m_page->view && ev->type() == QEvent::Show) {
        updateView();
    }

    if (o == m_page->view && ev->type() == QEvent::Resize) {
        m_buffer.resize(m_page->view->size());
    }

    if (o == m_page->view && ev->type() == QEvent::Paint) {
        updateVisibleArea();
    }

    if (o == m_page->view && ev->type() == QEvent::MouseMove) {
        QMouseEvent* me = (QMouseEvent*)ev;
        if (me->state() == LeftButton)
            handleMouseMoveAction(me->pos());
        else
        handleMouseMove(me->pos());

        m_lastPos = me->pos();
        return true;
    }

    if (o == m_page->view && ev->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = (QMouseEvent*)ev;
        if (me->button() == LeftButton) {
            handleMousePress(me->pos());
        }
        return true;
    }

    return m_page->eventFilter(o, ev);
}


void KoBirdEyePanel::handleMouseMove(QPoint p)
{
    m_handlePress = true;

    QRect r1 = QRect(m_visibleArea.x()-1, m_visibleArea.y()-1, 3, m_visibleArea.height()+2);
    if (r1.contains(p)) {
        m_page->view->setCursor(sizeHorCursor);
        m_aPos = AlignLeft;
        return;
    }

    r1.moveBy(m_visibleArea.width(),0);
    if (r1.contains(p)) {
        m_page->view->setCursor(sizeHorCursor);
        m_aPos = AlignRight;
        return;
    }

    QRect r2 = QRect(m_visibleArea.x()-1, m_visibleArea.y()-1, m_visibleArea.width()+2, 3);
    if (r2.contains(p)) {
        m_page->view->setCursor(sizeVerCursor);
        m_aPos = AlignTop;
        return;
    }

    r2.moveBy(0, m_visibleArea.height());
    
    if (r2.contains(p)) {
        m_page->view->setCursor(sizeVerCursor);
        m_aPos = AlignBottom;
        return;
    }

    if (m_visibleArea.contains(p)) {
        m_page->view->setCursor(sizeAllCursor);
        m_aPos = AlignCenter;
        return;
    }

    m_page->view->setCursor(arrowCursor);
    m_handlePress = false;
    
}

void KoBirdEyePanel::handleMouseMoveAction(QPoint p)
{
    if (!m_handlePress)
        return;

    p -= m_lastPos;

#if 0
    if (m_aPos == AlignCenter) {
        double zy = m_pView->zoomHandler()->zoomedResolutionY() / m_zoomHandler->zoomedResolutionY();
        double zx = m_pView->zoomHandler()->zoomedResolutionX() / m_zoomHandler->zoomedResolutionX();
        m_pCanvas->setUpdatesEnabled(false);
        m_pCanvas->scrollDx(-(int)(p.x()*zx));
        m_pCanvas->scrollDy(-(int)(p.y()*zy));
        m_pCanvas->setUpdatesEnabled(true);
        return;
    }
    double dx = m_zoomHandler->unzoomItX(p.x());
    double dy = m_zoomHandler->unzoomItY(p.y());

    QRect vr = m_canvas->visibleArea();
    if (m_aPos == AlignRight) {
        vr.setWidth(QMAX(10.0, vr.width() + dx));
        m_canvas->setVisibleAreaByWidth(vr);
    }
    else if (m_aPos == AlignLeft) {
        vr.setX(vr.x() + dx);
        vr.setWidth(QMAX(10.0, vr.width() - dx));
        m_pCanvas->setVisibleAreaByWidth(vr);
    }
    else if (m_aPos == AlignTop) {
        vr.setY(vr.y() + dy);
        vr.setHeight(QMAX(10.0 ,vr.height() - dy));
        m_pCanvas->setVisibleAreaByHeight(vr);
    }
    else if (m_aPos == AlignBottom) {
        vr.setHeight(QMAX(10.0 ,vr.height() + dy));
        m_pCanvas->setVisibleAreaByHeight(vr);
    }

#endif
    
}

void KoBirdEyePanel::handleMousePress(QPoint p)
{
    if (m_handlePress)
        return;
#if 0
    QSize s1 = m_page->view->size();
    KoPageLayout pl = m_pView->activePage()->paperLayout();
    int pw = m_zoomHandler->zoomItX(pl.ptWidth);
    int ph = m_zoomHandler->zoomItY(pl.ptHeight);
    int px0 = (s1.width()-pw)/2;
    int py0 = (s1.height()-ph)/2;

    double x = m_zoomHandler->unzoomItX(p.x() - px0);
    double y = m_zoomHandler->unzoomItY(p.y() - py0);

    m_canvas->setViewCenterPoint(KoPoint(x,y));
#endif    
}


#include "kobirdeyepanel.moc"
