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

KisColorSelectorBase::KisColorSelectorBase(QWidget *parent) :
    QWidget(parent),
    m_popup(0),
    m_hideDistance(40),
    m_timer(new QTimer(this))
{
    m_timer->setInterval(350);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hidePopup()));
}

KisColorSelectorBase::~KisColorSelectorBase()
{
    if(m_popup!=0)
        delete m_popup;
}

void KisColorSelectorBase::mousePressEvent(QMouseEvent* event)
{
    if((event->buttons()&(Qt::RightButton|Qt::MidButton))>0 && parent()!=0) {
        //open popup
        if(m_popup==0) {
            m_popup = createPopup();
            Q_ASSERT(m_popup);
            m_popup->setWindowFlags(Qt::Popup);
            m_popup->setMouseTracking(true);
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
    }
    else {
        //pick color
        int x = event->x();
        int y = event->y();

        x = qBound(0, x, width());
        y = qBound(0, y, height());
        pickColorAt(x, y);
    }
    event->accept();
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
    else {
        m_timer->stop();
    }

    QWidget::mouseMoveEvent(e);
}

void KisColorSelectorBase::hidePopup()
{
    hide();
}
