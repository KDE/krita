/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#include "kis_color_selector_base.h"

#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include <KDebug>
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_node.h"

KisColorSelectorBase::KisColorSelectorBase(QWidget *parent) :
    QWidget(parent),
    m_popup(0),
    m_hideDistance(40),
    m_timer(new QTimer(this)),
    m_popupOnMouseOver(false),
    m_popupOnMouseClick(true),
    m_colorSpace(0),
    m_canvas(0)
{
    m_timer->setInterval(350);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hidePopup()));

    if(parent==0 || m_popupOnMouseOver)
        setMouseTracking(true);
}

KisColorSelectorBase::~KisColorSelectorBase()
{
    if(m_popup!=0)
        delete m_popup;
}

void KisColorSelectorBase::setPopupBehaviour(bool onMouseOver, bool onMouseClick)
{
    m_popupOnMouseClick = onMouseClick;
    m_popupOnMouseOver = onMouseOver;
    if(onMouseClick) {
        m_popupOnMouseOver = false;
    }
    setMouseTracking(false);
    if(m_popupOnMouseOver) {
        setMouseTracking(true);
    }
}

void KisColorSelectorBase::setColorSpace(const KoColorSpace *colorSpace)
{
    m_colorSpace = colorSpace;
}

void KisColorSelectorBase::setCanvas(KisCanvas2 *canvas)
{
    m_canvas = canvas;

//    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
//            this,                        SLOT(resourceChanged(int, const QVariant&)));
//    setColor(m_canvas->resourceManager()->foregroundColor().toQColor());

    update();
}

void KisColorSelectorBase::mousePressEvent(QMouseEvent* event)
{
    if(m_popupOnMouseClick && (event->buttons()&Qt::MidButton)>0 && parent()!=0) {
        //open popup
        if(m_popup==0) {
            m_popup = createPopup();
            Q_ASSERT(m_popup);
            m_popup->setWindowFlags(Qt::Popup);
            m_popup->setCanvas(m_canvas);
        }

        int x = event->globalX();
        int y = event->globalY();
        int popupsize = m_popup->width();
        x-=popupsize/2;
        y-=popupsize/2;

        QRect availRect = QApplication::desktop()->availableGeometry(this);
        if(x<availRect.x())
            x = availRect.x();
        if(y<availRect.y())
            y = availRect.y();
        if(x+m_popup->width()>availRect.x()+availRect.width())
            x = availRect.x()+availRect.width()-m_popup->width();
        if(y+m_popup->height()>availRect.y()+availRect.height())
            y = availRect.y()+availRect.height()-m_popup->height();


        m_popup->move(x, y);
        m_popup->show();

        event->accept();
    }
    else if(parent() == 0 && (event->buttons()&Qt::MidButton)>0) {
        event->accept();
        hide();
    }
    else {
        event->ignore();
    }
}

void KisColorSelectorBase::mouseMoveEvent(QMouseEvent* e)
{
    if(parent()==0 && (qMin(e->x(), e->y())<-m_hideDistance || qMax(e->x(), e->y())>width()+m_hideDistance)) {
        if(!m_timer->isActive()) {
            m_timer->start();
        }
        e->accept();
        return;
    }
    else if (parent()==0){
        m_timer->stop();
        e->accept();
        return;
    }
    else if(parent()!=0 && m_popupOnMouseOver && this->rect().contains(e->pos()) && (m_popup==0 || m_popup->isHidden())) {
        //open popup
        if(m_popup==0) {
            m_popup = createPopup();
            Q_ASSERT(m_popup);
            m_popup->setWindowFlags(Qt::Popup);
            m_popup->setCanvas(m_canvas);
        }

        QRect availRect = QApplication::desktop()->availableGeometry(this);
        QRect forbiddenRect = QRect(parentWidget()->mapToGlobal(QPoint(0,0)),
                                    parentWidget()->mapToGlobal(QPoint(parentWidget()->width(), parentWidget()->height())));

//        kDebug()<<"availRect="<<availRect;
//        kDebug()<<"forbiddenRect="<<forbiddenRect;
//        kDebug()<<"popup="<<m_popup->geometry();

        int x,y;
        if(forbiddenRect.y()+forbiddenRect.height()/2 > availRect.height()/2) {
            //popup above forbiddenRect
            y = forbiddenRect.y()-m_popup->height();
        }
        else {
            //popup below forbiddenRect
            y = forbiddenRect.y()+forbiddenRect.height();
        }

        if(forbiddenRect.x()+forbiddenRect.width()/2 < availRect.width()/2) {
            //left edge of popup justified with left edge of popup
            x = forbiddenRect.x();
            kDebug()<<"1 forbiddenRect.x="<<forbiddenRect.x();
        }
        else {
            //the other way round
            x = forbiddenRect.x()+forbiddenRect.width()-m_popup->width();
            kDebug()<<"2 forbiddenRect.x="<<m_popup->width();
        }

        m_popup->move(x, y);
        m_popup->show();
        e->accept();
        return;
    }
}

void KisColorSelectorBase::setColor(const QColor& color)
{
    Q_UNUSED(color);
}

void KisColorSelectorBase::hidePopup()
{
    if(parent()==0)
        hide();
    else if (m_popup!=0)
        m_popup->hide();
}

void KisColorSelectorBase::commitColor(const KoColor& color, const QColor& rawColor, ColorRole role)
{
    Q_ASSERT(m_canvas);
    if (!m_canvas)
        return;

    if (role==Foreground)
        m_canvas->resourceManager()->setForegroundColor(KoColor(color , colorSpace()));
    else
        m_canvas->resourceManager()->setBackgroundColor(KoColor(color , colorSpace()));

    emit colorChanged(rawColor);
}

//void KisColorSelectorBase::resourceChanged(int key, const QVariant &v)
//{
//    if (key == KoCanvasResource::ForegroundColor || key == KoCanvasResource::BackgroundColor) {
////        KoColor kc(v.value<KoColor>().data(), KoColorSpaceRegistry::instance()->rgb8());
////        setColor(kc.toQColor());
////        setColor(kc.toQColor());
////        setColor(v.value<KoColor>().toQColor());
//    }
//}

const KoColorSpace* KisColorSelectorBase::colorSpace()
{
    if(m_colorSpace!=0) {
        return m_colorSpace;
    }
    else {
        Q_ASSERT(m_canvas);
        KisNodeSP currentNode = m_canvas->resourceManager()->
                                resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
        m_colorSpace=currentNode->colorSpace();
        return m_colorSpace;
//        return m_canvas->currentImage()->colorSpace();
    }
}

void KisColorSelectorBase::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    if(cfg.readEntry("useCustomColorSpace", true)) {
        KoColorSpaceRegistry* csr = KoColorSpaceRegistry::instance();
        m_colorSpace = csr->colorSpace(cfg.readEntry("customColorSpaceModel", "RGBA"),
                                                         cfg.readEntry("customColorSpaceDepthID", "U8"),
                                                         cfg.readEntry("customColorSpaceProfile", "sRGB built-in - (lcms internal)"));
    }
    else {
        m_colorSpace=0;
        // the colorspace will be retrieved next time by calling colorSpace()
    }
}
