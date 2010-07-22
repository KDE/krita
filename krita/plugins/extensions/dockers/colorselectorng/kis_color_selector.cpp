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
#include "kis_color_selector_wheel.h"

#include <cmath>

#include <QHBoxLayout>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include <KDebug>

KisColorSelector::KisColorSelector(Configuration conf, QWidget* parent)
                                       : KisColorSelectorBase(parent),
                                       m_ring(0),
                                       m_triangle(0),
                                       m_slider(0),
                                       m_square(0),
                                       m_wheel(0),
                                       m_mainComponent(0),
                                       m_subComponent(0)
{
    init();
    setConfiguration(conf);
}

KisColorSelector::KisColorSelector(QWidget* parent)
                                       : KisColorSelectorBase(parent),
                                       m_ring(0),
                                       m_triangle(0),
                                       m_slider(0),
                                       m_square(0),
                                       m_wheel(0),
                                       m_mainComponent(0),
                                       m_subComponent(0)
{
    init();
    updateSettings();
}

KisColorSelectorBase* KisColorSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisColorSelector(0);
    popup->resize(350,350);
    return popup;
}

void KisColorSelector::setConfiguration(Configuration conf)
{
    m_configuration = conf;

    if(m_mainComponent!=0) {
        Q_ASSERT(m_subComponent!=0);
        m_mainComponent->setGeometry(0, 0, 0, 0);
        m_subComponent->setGeometry(0, 0, 0, 0);

        m_mainComponent->disconnect();
        m_subComponent->disconnect();
    }

    switch (m_configuration.mainType) {
    case Square:
        m_mainComponent=m_square;
        break;
    case Wheel:
        m_mainComponent=m_wheel;
        break;
    case Triangle:
        m_mainComponent=m_triangle;
        break;
    default:
        Q_ASSERT(false);
    }

    switch (m_configuration.subType) {
    case Ring:
        m_subComponent=m_ring;
        break;
    case Slider:
        m_subComponent=m_slider;
        break;
    default:
        Q_ASSERT(false);
    }

    connect(m_mainComponent, SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal)),
            m_subComponent,  SLOT(setParam(qreal,qreal,qreal,qreal,qreal)), Qt::UniqueConnection);
    connect(m_subComponent,  SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal)),
            m_mainComponent, SLOT(setParam(qreal,qreal,qreal,qreal, qreal)), Qt::UniqueConnection);

    connect(m_mainComponent, SIGNAL(update()), m_updateTimer,   SLOT(start()), Qt::UniqueConnection);
    connect(m_subComponent,  SIGNAL(update()), m_updateTimer,   SLOT(start()), Qt::UniqueConnection);
    
    QResizeEvent event(QSize(width(),
                             height()),
                       QSize());
    resizeEvent(&event);
    setColor(QColor(255,0,0));
}

KisColorSelector::Configuration KisColorSelector::configuration() const
{
    return m_configuration;
}

void KisColorSelector::updateSettings()
{
    KisColorSelectorBase::updateSettings();
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    setConfiguration(Configuration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelector::Configuration().toString())));
}

void KisColorSelector::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.fillRect(0,0,width(),height(),QColor(128,128,128));
    p.setRenderHint(QPainter::Antialiasing);

    m_mainComponent->paintEvent(&p);
    m_subComponent->paintEvent(&p);
}

void KisColorSelector::resizeEvent(QResizeEvent* e) {
    if(m_configuration.subType==Ring) {
        m_ring->setGeometry(0,0,width(), height());
        if(m_configuration.mainType==Triangle) {
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
        }
    }
    else {
        // type wheel and square
        m_mainComponent->setGeometry(0,height()*0.1,width(), height()*0.9);
        m_subComponent->setGeometry(0,0,width(), height()*0.1);
    }
    m_mainComponent->setConfiguration(m_configuration.mainTypeParameter, m_configuration.mainType);
    m_subComponent->setConfiguration(m_configuration.subTypeParameter, m_configuration.subType);

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

void KisColorSelector::mouseReleaseEvent(QMouseEvent* e)
{
    if(m_lastColor!=m_currentColor && m_currentColor.isValid()) {
        m_lastColor=m_currentColor;
        ColorRole role;
        if(e->button() == Qt::LeftButton)
            role=Foreground;
        else
            role=Background;
        commitColor(m_currentColor, role);
        e->accept();
    }
}

void KisColorSelector::setColor(const QColor &color)
{
//    m_ring->mouseEvent(-1,-1);
//    m_triangle->setParam(color.hueF());
//    m_square->setColor(color);
//    m_slider->setColor(color);
//    update();
}

void KisColorSelector::mouseEvent(QMouseEvent *e)
{
    if(m_lastMousePosition==e->pos())
        return;
    m_lastMousePosition=e->pos();

    if(e->buttons()&Qt::LeftButton || e->buttons()&Qt::RightButton) {
        m_mainComponent->mouseEvent(e->x(), e->y());
        m_subComponent->mouseEvent(e->x(), e->y());

        m_currentColor=m_mainComponent->currentColor();
    }
}

void KisColorSelector::init()
{
    m_ring = new KisColorSelectorRing(this);
    m_triangle = new KisColorSelectorTriangle(this);
    m_slider = new KisColorSelectorSimple(this);
    m_square = new KisColorSelectorSimple(this);
    m_wheel = new KisColorSelectorWheel(this);

    // a tablet can send many more signals, than a mouse
    // this causes many repaints, if updating after every signal.
    // a workaround with a timer can fix that.
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1);
    m_updateTimer->setSingleShot(true);

    connect(m_updateTimer,      SIGNAL(timeout()), this,  SLOT(update()));

    setMinimumSize(80, 80);

    m_lastMousePosition=QPoint(-1, -1);
}
