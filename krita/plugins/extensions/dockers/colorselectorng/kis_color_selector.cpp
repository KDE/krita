/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_selector.h"

#include "kis_color_selector_ring.h"
#include "kis_color_selector_triangle.h"
#include "kis_color_selector_simple.h"

#include <cmath>

#include <QHBoxLayout>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>


#include <KDebug>

KisColorSelector::KisColorSelector(QWidget* parent,
                                   Type mainType,
                                   Type subType,
                                   Parameters mainTypeParam,
                                   Parameters subTypeParam)
                                       : KisColorSelectorBase(parent),
                                       m_ring(0),
                                       m_triangle(0),
                                       m_slider(0),
                                       m_square(0)
{
    m_ring = new KisColorSelectorRing(this);
    m_triangle = new KisColorSelectorTriangle(this);
    m_slider = new KisColorSelectorSimple(this);
    m_square = new KisColorSelectorSimple(this);
    
    connect(m_ring,     SIGNAL(update()), this, SLOT(update()));
    connect(m_triangle, SIGNAL(update()), this, SLOT(update()));
    connect(m_slider,   SIGNAL(update()), this, SLOT(update()));
    connect(m_square,   SIGNAL(update()), this, SLOT(update()));

    connect(m_ring, SIGNAL(paramChanged(qreal)), m_triangle, SLOT(setParam(qreal)));
    connect(m_ring, SIGNAL(paramChanged(qreal)), m_square,   SLOT(setParam(qreal)));

    connect(m_square, SIGNAL(paramChanged(qreal,qreal)), m_slider, SLOT(setParam(qreal,qreal)));
    connect(m_slider, SIGNAL(paramChanged(qreal)),       m_square, SLOT(setParam(qreal)));
    
    setMinimumSize(80, 80);

    setConfiguration(mainType, subType, mainTypeParam, subTypeParam);
}

KisColorSelectorBase* KisColorSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisColorSelector(0);
    popup->resize(350,350);
    return popup;
}

void KisColorSelector::setConfiguration(Type mainType, Type subType, Parameters mainTypeParam, Parameters subTypeParam)
{
    m_mainType = mainType;
    m_subType = subType;
    m_mainTypeParam = mainTypeParam;
    m_subTypeParam = subTypeParam;

    QResizeEvent event(QSize(width(),
                             height()),
                       QSize());
    resizeEvent(&event);
}

void KisColorSelector::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.fillRect(0,0,width(),height(),QColor(128,128,128));
    p.setRenderHint(QPainter::Antialiasing);

    if(m_ring->width()>0)
        m_ring->paintEvent(&p);
    if(m_triangle->width()>0)
        m_triangle->paintEvent(&p);
    if(m_slider->width()>0)
        m_slider->paintEvent(&p);
    if(m_square->width()>0)
        m_square->paintEvent(&p);
}

void KisColorSelector::resizeEvent(QResizeEvent* e) {
    m_ring->setGeometry(0,0,0,0);
    m_triangle->setGeometry(0,0,0,0);
    m_slider->setGeometry(0,0,0,0);
    m_square->setGeometry(0,0,0,0);

    if(m_mainType==Ring) {
        m_ring->setGeometry(0,0,width(), height());
        if(m_subType==Triangle) {
            m_triangle->setGeometry(width()/2-m_ring->innerRadius(),
                                    height()/2-m_ring->innerRadius(),
                                    m_ring->innerRadius()*2,
                                    m_ring->innerRadius()*2);
        }
        else {
            int size = m_ring->innerRadius()*2/sqrt(2.);
            m_square->setGeometry(width()/2-size/2,
                                  height()/2-size/2,
                                  size,
                                  size);
            m_square->setConfiguration(m_subTypeParam, m_subType);
        }
    }
    else {
        // type wheel and square
        m_square->setGeometry(0,height()*0.1,width(), height()*0.9);
        m_square->setConfiguration(m_mainTypeParam, m_mainType);
        m_slider->setGeometry(0,0,width(), height()*0.1);
        m_slider->setConfiguration(m_subTypeParam, m_subType);
    }

    KisColorSelectorBase::resizeEvent(e);
}

void KisColorSelector::mousePressEvent(QMouseEvent* e)
{
    KisColorSelectorBase::mousePressEvent(e);

    if(!e->isAccepted()) {
        mouseEvent(e);
    }
}

void KisColorSelector::mouseMoveEvent(QMouseEvent* e)
{
    KisColorSelectorBase::mouseMoveEvent(e);
    mouseEvent(e);
}

void KisColorSelector::mouseEvent(QMouseEvent *e)
{
    if(e->buttons()&Qt::LeftButton || e->buttons()&Qt::RightButton) {
        m_ring->mouseEvent(e->x(), e->y());
        m_triangle->mouseEvent(e->x(), e->y());
        if(m_lastColor==m_triangle->currentColor()) {
            m_lastColor=m_triangle->currentColor();
            ColorRole role;
            if(e->buttons() & Qt::LeftButton)
                role=Foreground;
            else
                role=Background;
            commitColor(m_triangle->currentColor(), role);
        }
    }
}
