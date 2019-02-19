/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
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
#include <QApplication>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <kis_debug.h>

#include <KoCanvasResourceProvider.h>
#include <kis_icon.h>

#include "kis_color_selector_ring.h"
#include "kis_color_selector_triangle.h"
#include "kis_color_selector_simple.h"
#include "kis_color_selector_wheel.h"
#include "kis_color_selector_container.h"
#include "kis_canvas2.h"
#include "kis_signal_compressor.h"
#include "KisViewManager.h"


KisColorSelector::KisColorSelector(KisColorSelectorConfiguration conf, QWidget* parent)
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
    updateSettings();
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
    popup->setColor(m_lastRealColor);
    return popup;
}

void KisColorSelector::setConfiguration(KisColorSelectorConfiguration conf)
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
    case KisColorSelectorConfiguration::Square:
        m_mainComponent=m_square;
        break;
    case KisColorSelectorConfiguration::Wheel:
        m_mainComponent=m_wheel;
        break;
    case KisColorSelectorConfiguration::Triangle:
        m_mainComponent=m_triangle;
        break;
    default:
        Q_ASSERT(false);
    }

    switch (m_configuration.subType) {
    case KisColorSelectorConfiguration::Ring:
        m_subComponent=m_ring;
        break;
    case KisColorSelectorConfiguration::Slider:
        m_subComponent=m_slider;
        break;
    default:
        Q_ASSERT(false);
    }

    connect(m_mainComponent, SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
            m_subComponent,  SLOT(setParam(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::UniqueConnection);
    connect(m_subComponent,  SIGNAL(paramChanged(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
            m_mainComponent, SLOT(setParam(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::UniqueConnection);

    connect(m_mainComponent, SIGNAL(update()), m_signalCompressor, SLOT(start()), Qt::UniqueConnection);
    connect(m_subComponent,  SIGNAL(update()), m_signalCompressor, SLOT(start()), Qt::UniqueConnection);

    m_mainComponent->setConfiguration(m_configuration.mainTypeParameter, m_configuration.mainType);
    m_subComponent->setConfiguration(m_configuration.subTypeParameter, m_configuration.subType);

    QResizeEvent event(QSize(width(), height()), QSize());
    resizeEvent(&event);
}

KisColorSelectorConfiguration KisColorSelector::configuration() const
{
    return m_configuration;
}

void KisColorSelector::updateSettings()
{
    KisColorSelectorBase::updateSettings();
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    setConfiguration(KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString())));
}

void KisColorSelector::slotGamutMaskSet(KoGamutMask *gamutMask)
{
    m_mainComponent->setGamutMask(gamutMask);
    m_subComponent->setGamutMask(gamutMask);

    slotGamutMaskToggle(true);
}

void KisColorSelector::slotGamutMaskUnset()
{
    m_mainComponent->unsetGamutMask();
    m_subComponent->unsetGamutMask();

    slotGamutMaskToggle(false);
}

void KisColorSelector::slotGamutMaskPreviewUpdate()
{
    m_mainComponent->updateGamutMaskPreview();
    m_subComponent->updateGamutMaskPreview();
}

void KisColorSelector::slotGamutMaskToggle(bool state)
{
    m_mainComponent->toggleGamutMask(state);
    m_subComponent->toggleGamutMask(state);
}

void KisColorSelector::updateIcons() {
    if (m_button) {
        m_button->setIcon(KisIconUtils::loadIcon("configure"));
    }
}

void KisColorSelector::hasAtLeastOneDocument(bool value)
{
    m_hasAtLeastOneDocumentOpen = value;
}

void KisColorSelector::reset()
{
    if (m_mainComponent) {
        m_mainComponent->setDirty();
    }

    if (m_subComponent) {
        m_subComponent->setDirty();
    }

    KisColorSelectorBase::reset();
}

void KisColorSelector::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter p(this);
    p.fillRect(0,0,width(), height(), QColor(128,128,128));
    p.setRenderHint(QPainter::Antialiasing);

    // this variable name isn't entirely accurate to what always happens. see definition in header file to understand it better
    if (!m_hasAtLeastOneDocumentOpen) {
        p.setOpacity(0.2);
    }

    m_mainComponent->paintEvent(&p);
    m_subComponent->paintEvent(&p);

    p.setOpacity(1.0);
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
    if (m_configuration.subType == KisColorSelectorConfiguration::Ring) {

        m_ring->setGeometry(0,0,width(), height());

        if (displaySettingsButton()) {
            int size = iconSize(width(), height());
            m_button->setGeometry(0, 0, size, size);
        }

        if (m_configuration.mainType == KisColorSelectorConfiguration::Triangle) {
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
        if (m_configuration.mainType == KisColorSelectorConfiguration::Wheel) {
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

    // reset the correct color after resizing the widget
    setColor(m_lastRealColor);

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
        updatePreviousColorPreview();
        e->accept();
    }
}

void KisColorSelector::mouseMoveEvent(QMouseEvent* e)
{
    KisColorSelectorBase::mouseMoveEvent(e);

    mouseEvent(e);
    e->accept();
}

void KisColorSelector::mouseReleaseEvent(QMouseEvent* e)
{
    e->setAccepted(false);
    KisColorSelectorBase::mouseReleaseEvent(e);

    if(!e->isAccepted() &&
       !(m_lastRealColor == m_currentRealColor)) {

        m_lastRealColor = m_currentRealColor;
        m_lastColorRole = Acs::buttonToRole(e->button());

        updateColor(m_lastRealColor, m_lastColorRole, false);
        updateBaseColorPreview(m_currentRealColor);
        e->accept();
    }

    m_grabbingComponent=0;
}

bool KisColorSelector::displaySettingsButton()
{
    return dynamic_cast<KisColorSelectorContainer*>(parent());
}

void KisColorSelector::setColor(const KoColor &color)
{
    m_mainComponent->setColor(color);
    m_subComponent->setColor(color);
    m_lastRealColor = color;

    m_signalCompressor->start();
}

void KisColorSelector::mouseEvent(QMouseEvent *e)
{
    if (m_grabbingComponent && (e->buttons() & Qt::LeftButton || e->buttons() & Qt::RightButton)) {

        m_grabbingComponent->mouseEvent(e->x(), e->y());

        KoColor color = m_mainComponent->currentColor();
        Acs::ColorRole role = Acs::buttonsToRole(e->button(), e->buttons());
        m_currentRealColor = color;

        requestUpdateColorAndPreview(color, role);
    }
}

void KisColorSelector::init()
{
    setAcceptDrops(true);

    m_lastColorRole = Acs::Foreground;
    m_ring = new KisColorSelectorRing(this);
    m_triangle = new KisColorSelectorTriangle(this);
    m_slider = new KisColorSelectorSimple(this);
    m_square = new KisColorSelectorSimple(this);
    m_wheel = new KisColorSelectorWheel(this);

    if(displaySettingsButton()) {
        m_button = new QPushButton(this);
        m_button->setIcon(KisIconUtils::loadIcon("configure"));
        m_button->setFlat(true);
        connect(m_button, SIGNAL(clicked()), SIGNAL(settingsButtonClicked()));
    }

    // a tablet can send many more signals, than a mouse
    // this causes many repaints, if updating after every signal.
    m_signalCompressor = new KisSignalCompressor(20, KisSignalCompressor::FIRST_INACTIVE, this);
    connect(m_signalCompressor, SIGNAL(timeout()), SLOT(update()));

    setMinimumSize(40, 40);
}
