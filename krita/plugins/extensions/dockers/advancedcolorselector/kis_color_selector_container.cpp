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

#include "kis_color_selector_container.h"

#include "kis_color_selector.h"
#include "kis_my_paint_shade_selector.h"
#include "kis_minimal_shade_selector.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>
#include <KAction>
#include <KActionCollection>

#include "kis_view2.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_node_manager.h"
#include "kis_node.h"
#include "kis_paint_device.h"

KisColorSelectorContainer::KisColorSelectorContainer(QWidget *parent) :
    QWidget(parent),
    m_colorSelector(new KisColorSelector(this)),
    m_myPaintShadeSelector(new KisMyPaintShadeSelector(this)),
    m_minimalShadeSelector(new KisMinimalShadeSelector(this)),
    m_shadeSelector(m_myPaintShadeSelector),
    m_shadeSelectorHideable(false),
    m_allowHorizontalLayout(true),
    m_colorSelAction(0),
    m_mypaintAction(0),
    m_minimalAction(0),
    m_canvas(0)
{
    m_widgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_widgetLayout->setSpacing(0);
    m_widgetLayout->setMargin(0);

    m_widgetLayout->addWidget(m_colorSelector);
    m_widgetLayout->addWidget(m_myPaintShadeSelector);
    m_widgetLayout->addWidget(m_minimalShadeSelector);

    m_myPaintShadeSelector->hide();
    m_minimalShadeSelector->hide();

    connect(m_colorSelector,SIGNAL(settingsButtonClicked()), SIGNAL(openSettings()));

    connect(this, SIGNAL(settingsChanged()), m_colorSelector,        SLOT(updateSettings()));
    connect(this, SIGNAL(settingsChanged()), m_myPaintShadeSelector, SLOT(updateSettings()));
    connect(this, SIGNAL(settingsChanged()), this,                   SLOT(updateSettings()));
    connect(this, SIGNAL(settingsChanged()), m_minimalShadeSelector, SLOT(updateSettings()));

}

void KisColorSelectorContainer::unsetCanvas()
{
    m_colorSelector->unsetCanvas();
    m_myPaintShadeSelector->unsetCanvas();
    m_minimalShadeSelector->unsetCanvas();
    m_canvas = 0;
}

void KisColorSelectorContainer::setCanvas(KisCanvas2 *canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->view()->nodeManager()->disconnect(this);
        KActionCollection *ac = m_canvas->view()->actionCollection();
        ac->takeAction(ac->action("show_color_selector"));
        ac->takeAction(ac->action("show_mypaint_shade_selector"));
        ac->takeAction(ac->action("show_minimal_shade_selector"));
    }

    m_canvas = canvas;

    m_colorSelector->setCanvas(canvas);
    m_myPaintShadeSelector->setCanvas(canvas);
    m_minimalShadeSelector->setCanvas(canvas);

    if (m_canvas->view()->nodeManager()) {
        connect(m_canvas->view()->nodeManager(), SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(reactOnLayerChange()), Qt::UniqueConnection);
    }
    KActionCollection* actionCollection = canvas->view()->actionCollection();

    if (!m_colorSelAction != 0) {
        m_colorSelAction = new KAction("Show color selector", this);
        m_colorSelAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_I));
        connect(m_colorSelAction, SIGNAL(triggered()), m_colorSelector, SLOT(showPopup()), Qt::UniqueConnection);
    }
    actionCollection->addAction("show_color_selector", m_colorSelAction);

    if (!m_mypaintAction) {
        m_mypaintAction = new KAction("Show MyPaint shade selector", this);
        m_mypaintAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_M));
        connect(m_mypaintAction, SIGNAL(triggered()), m_myPaintShadeSelector, SLOT(showPopup()), Qt::UniqueConnection);
    }
    actionCollection->addAction("show_mypaint_shade_selector", m_mypaintAction);

    if (!m_minimalAction) {
        m_minimalAction = new KAction("Show minimal shade selector", this);
        m_minimalAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_N));
        connect(m_minimalAction, SIGNAL(triggered()), m_minimalShadeSelector, SLOT(showPopup()), Qt::UniqueConnection);
    }
    actionCollection->addAction("show_minimal_shade_selector", m_minimalAction);

}

void KisColorSelectorContainer::updateSettings()
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    m_shadeSelectorHideable = cfg.readEntry("shadeSelectorHideable", false);
    m_allowHorizontalLayout = cfg.readEntry("allowHorizontalLayout", true);

    QString type = cfg.readEntry("shadeSelectorType", "MyPaint");

    QWidget* newShadeSelector;
    if(type=="MyPaint")
        newShadeSelector = m_myPaintShadeSelector;
    else if (type=="Minimal")
        newShadeSelector = m_minimalShadeSelector;
    else
        newShadeSelector = 0;

    if(m_shadeSelector!=newShadeSelector && m_shadeSelector!=0) {
        m_shadeSelector->hide();
    }
    m_shadeSelector=newShadeSelector;

    if(m_shadeSelector!=0)
        m_shadeSelector->show();

}

void KisColorSelectorContainer::reactOnLayerChange()
{
    if (m_canvas) {
        KisNodeSP node = m_canvas->view()->resourceProvider()->currentNode();
        if (node) {
            KisPaintDeviceSP device = node->paintDevice();
            if (device) {
                m_colorSelAction->setEnabled(true);
                m_mypaintAction->setEnabled(true);
                m_minimalAction->setEnabled(true);
            }
            else {
                //            m_colorSelAction->setEnabled(false);
                //            m_mypaintAction->setEnabled(false);
                //            m_minimalAction->setEnabled(false);
            }
        }
    }
}

void KisColorSelectorContainer::resizeEvent(QResizeEvent * e)
{
    if(m_shadeSelector!=0) {
        int minimumHeightForBothWidgets = m_colorSelector->minimumHeight()+m_shadeSelector->minimumHeight()+30; //+30 for the buttons (temporarily)

        if(height()<minimumHeightForBothWidgets && m_shadeSelectorHideable) {
            m_shadeSelector->hide();
        }
        else {
            m_shadeSelector->show();
        }
        if(height() < width() && m_allowHorizontalLayout && m_shadeSelector!=m_minimalShadeSelector) {
            m_widgetLayout->setDirection(QBoxLayout::LeftToRight);
        }
        else {
            m_widgetLayout->setDirection(QBoxLayout::TopToBottom);
        }
    }

    QWidget::resizeEvent(e);
}
