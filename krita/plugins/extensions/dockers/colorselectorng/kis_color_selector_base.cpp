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

#include <KDebug>
#include "KoColorSpace.h"

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

void KisColorSelectorBase::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(m_canvas);
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

    QWidget::mouseMoveEvent(e);
}

void KisColorSelectorBase::hidePopup()
{
    if(parent()==0)
        hide();
    else if (m_popup!=0)
        m_popup->hide();
}

void KisColorSelectorBase::commitColor(QColor color, ColorRole role)
{

}

const KoColorSpace* KisColorSelectorBase::colorSpace() const
{
    Q_ASSERT(m_canvas);
    if(m_colorSpace!=0) {
        return m_colorSpace;
    }
    else {
        KisNodeSP currentNode = m_canvas->resourceManager()->
                                resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
        return currentNode->colorSpace();
//        return m_canvas->currentImage()->colorSpace();
    }
}
