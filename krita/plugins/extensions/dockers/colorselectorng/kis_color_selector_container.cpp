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
#include <KIcon>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include <KDebug>

KisColorSelectorContainer::KisColorSelectorContainer(QWidget *parent) :
    QWidget(parent),
    m_colorSelector(new KisColorSelector(this)),
    m_myPaintShadeSelector(new KisMyPaintShadeSelector(this)),
    m_minimalShadeSelector(new KisMinimalShadeSelector(this)),
    m_shadeSelector(m_myPaintShadeSelector),
    m_shadeSelectorHideable(false),
    m_allowHorizontalLayout(true)
{
    QPushButton* pipetteButton = new QPushButton(this);
    QPushButton* settingsButton = new QPushButton(this);

    pipetteButton->setIcon(KIcon("krita_tool_color_picker"));
    pipetteButton->setFlat(true);
    settingsButton->setIcon(KIcon("configure"));
    settingsButton->setFlat(true);
    pipetteButton->setMaximumWidth(24);
    settingsButton->setMaximumWidth(24);

    m_buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    m_buttonLayout->addStretch(1);
    m_buttonLayout->addWidget(pipetteButton);
    m_buttonLayout->addWidget(settingsButton);
    m_buttonLayout->setMargin(0);
    m_buttonLayout->setSpacing(0);

    m_widgetLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_widgetLayout->setSpacing(0);
    m_widgetLayout->setMargin(0);

    m_widgetLayout->addLayout(m_buttonLayout);
    m_widgetLayout->addWidget(m_colorSelector);
    m_widgetLayout->addWidget(m_myPaintShadeSelector);
    m_widgetLayout->addWidget(m_minimalShadeSelector);

    m_myPaintShadeSelector->hide();
    m_minimalShadeSelector->hide();

    connect(settingsButton, SIGNAL(clicked()),         this,                   SIGNAL(openSettings()));
    connect(this,           SIGNAL(settingsChanged()), m_colorSelector,        SLOT(updateSettings()));
    connect(this,           SIGNAL(settingsChanged()), m_myPaintShadeSelector, SLOT(updateSettings()));
    connect(this,           SIGNAL(settingsChanged()), this,                   SLOT(updateSettings()));
    connect(this,           SIGNAL(settingsChanged()), m_minimalShadeSelector, SLOT(updateSettings()));

    connect(m_colorSelector,        SIGNAL(colorChanged(const QColor&)), m_myPaintShadeSelector, SLOT(setColor(const QColor&)));
    connect(m_myPaintShadeSelector, SIGNAL(colorChanged(const QColor&)), m_colorSelector,        SLOT(setColor(const QColor&)));
    connect(m_colorSelector,        SIGNAL(colorChanged(const QColor&)), m_minimalShadeSelector, SLOT(setColor(const QColor&)));
    connect(m_myPaintShadeSelector, SIGNAL(colorChanged(const QColor&)), m_minimalShadeSelector, SLOT(setColor(const QColor&)));
}

void KisColorSelectorContainer::setCanvas(KisCanvas2 *canvas)
{
    m_colorSelector->setCanvas(canvas);
    m_myPaintShadeSelector->setCanvas(canvas);
    m_minimalShadeSelector->setCanvas(canvas);
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
        if(height() < width() && m_allowHorizontalLayout) {
            m_widgetLayout->setDirection(QBoxLayout::LeftToRight);
            m_buttonLayout->setDirection(QBoxLayout::BottomToTop);
        }
        else {
            m_widgetLayout->setDirection(QBoxLayout::TopToBottom);
            m_buttonLayout->setDirection(QBoxLayout::LeftToRight);
        }
    }

    QWidget::resizeEvent(e);
}
