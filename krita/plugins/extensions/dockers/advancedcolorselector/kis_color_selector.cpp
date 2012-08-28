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

#include <cmath>

#include <QHBoxLayout>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>
#include <KDebug>

#include <KoCanvasResourceManager.h>
#include <KoIcon.h>

#include "kis_color_selector_ring.h"
#include "kis_color_selector_triangle.h"
#include "kis_color_selector_simple.h"
#include "kis_color_selector_wheel.h"
#include "kis_color_selector_container.h"
#include "kis_canvas2.h"

KisColorSelector::KisColorSelector(Configuration conf, QWidget* parent)
    : KisColorSelectorBase(parent),
      m_ring(0),
      m_triangle(0),
      m_slider(0),
      m_square(0),
      m_wheel(0),
      m_mainComponent(0),
      m_subComponent(0),
      m_grabbingComponent(0),
      m_blipDisplay(true)
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
      m_button(0),
      m_mainComponent(0),
      m_subComponent(0),
      m_grabbingComponent(0),
      m_blipDisplay(true)
{
    init();
    updateSettings();
}

KisColorSelectorBase* KisColorSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisColorSelector(0);
    popup->setColor(m_lastColor);
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
    
    m_mainComponent->setConfiguration(m_configuration.mainTypeParameter, m_configuration.mainType);
    m_subComponent->setConfiguration(m_configuration.subTypeParameter, m_configuration.subType);

    QResizeEvent event(QSize(width(), height()), QSize());
    resizeEvent(&event);
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

inline int iconSize(qreal width, qreal height) {
    qreal radius = qMin(width, height)/2.;
    qreal xm = width/2.;
    qreal ym = height/2.;
    if(xm>=2*ym || ym>=2*xm)
        return qBound<qreal>(5., radius, 32.);

    qreal a=-2;
    qreal b=2.*(xm+ym);
    qreal c=radius*radius-xm*xm-ym*ym;
    return qBound<qreal>(5., ((-b+sqrt(b*b-4*a*c))/(2*a)), 32.);
}

void KisColorSelector::resizeEvent(QResizeEvent* e) {
    if(m_configuration.subType==Ring) {
        m_ring->setGeometry(0,0,width(), height());
        if(displaySettingsButton()) {
            int size = iconSize(width(), height());
            m_button->setGeometry(0, 0, size, size);
        }
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
        if(m_configuration.mainType==Wheel) {
            if(displaySettingsButton()) {
                int size = iconSize(width(), height()*0.9);
                m_button->setGeometry(0, height()*0.1, size, size);
            }
            m_mainComponent->setGeometry(0, height()*0.1, width(), height()*0.9);
            m_subComponent->setGeometry( 0, 0,            width(), height()*0.1);
        }
        else {
            int buttonSize = 0;
            if(displaySettingsButton()) {
                buttonSize = qBound(20, int(0.1*height()), 32);
                m_button->setGeometry(0, 0, buttonSize, buttonSize);
            }

            if(height()>width()) {
                int selectorHeight=height()-buttonSize;
                m_mainComponent->setGeometry(0, buttonSize+selectorHeight*0.1, width(), selectorHeight*0.9);
                m_subComponent->setGeometry( 0, buttonSize,                    width(), selectorHeight*0.1);
            }
            else {
                int selectorWidth=width()-buttonSize;
                m_mainComponent->setGeometry(buttonSize, height()*0.1, selectorWidth, height()*0.9);
                m_subComponent->setGeometry( buttonSize, 0,            selectorWidth, height()*0.1);
            }
        }
    }
    if(m_canvas) {
        if (m_lastColorRole==Foreground) {
            setColor(m_canvas->resourceManager()->foregroundColor().toQColor());
        } else {
            setColor(m_canvas->resourceManager()->backgroundColor().toQColor());
        }
    }
    KisColorSelectorBase::resizeEvent(e);
}

void KisColorSelector::mousePressEvent(QMouseEvent* e)
{
    e->setAccepted(false);
    KisColorSelectorBase::mousePressEvent(e);

    if(!e->isAccepted()) {
        if(m_mainComponent->wantsGrab(e->x(), e->y()))
            m_grabbingComponent=m_mainComponent;
        else if(m_subComponent->wantsGrab(e->x(), e->y()))
            m_grabbingComponent=m_subComponent;

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
    KisColorSelectorBase::mouseReleaseEvent(e);
    if(m_lastColor!=m_currentColor && m_currentColor.isValid()) {
        m_lastColor=m_currentColor;
        if(e->button() == Qt::LeftButton)
            m_lastColorRole=Foreground;
        else
            m_lastColorRole=Background;
        commitColor(KoColor(m_currentColor, colorSpace()), m_lastColorRole);

        if(isPopup() && m_mainComponent->containsPoint(e->pos())) {
            hidePopup();
        }
    }
    e->accept();
    m_grabbingComponent=0;
}

bool KisColorSelector::displaySettingsButton()
{
    if(dynamic_cast<KisColorSelectorContainer*>(parent())!=0)
        return true;
    else
        return false;
}

void KisColorSelector::setColor(const QColor &color)
{
    m_mainComponent->setColor(color);
    m_subComponent->setColor(color);
    m_lastColor=color;
    update();
}

void KisColorSelector::mouseEvent(QMouseEvent *e)
{
    if(m_grabbingComponent && (e->buttons()&Qt::LeftButton || e->buttons()&Qt::RightButton)) {
        m_grabbingComponent->mouseEvent(e->x(), e->y());

        m_currentColor=m_mainComponent->currentColor();
        KoColor kocolor(m_currentColor, colorSpace());
        updateColorPreview(kocolor.toQColor());
    }
}

void KisColorSelector::init()
{
    setAcceptDrops(true);

    m_ring = new KisColorSelectorRing(this);
    m_triangle = new KisColorSelectorTriangle(this);
    m_slider = new KisColorSelectorSimple(this);
    m_square = new KisColorSelectorSimple(this);
    m_wheel = new KisColorSelectorWheel(this);

    if(displaySettingsButton()) {
        m_button = new QPushButton(this);
        m_button->setIcon(koIcon("configure"));
        connect(m_button, SIGNAL(clicked()), SIGNAL(settingsButtonClicked()));
    }

    // a tablet can send many more signals, than a mouse
    // this causes many repaints, if updating after every signal.
    // a workaround with a timer can fix that.
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1);
    m_updateTimer->setSingleShot(true);

    connect(m_updateTimer,      SIGNAL(timeout()), this,  SLOT(update()));

    setMinimumSize(40, 40);
}
