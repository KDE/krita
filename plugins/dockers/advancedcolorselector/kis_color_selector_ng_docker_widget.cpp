/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_selector_ng_docker_widget.h"
#include "ui_wdg_color_selector_settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kis_icon_utils.h>

#include <QAction>
#include <kactioncollection.h>

#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_color_space_selector.h"
#include "kis_preference_set_registry.h"
#include "kis_node.h"
#include "kis_paint_device.h"

#include "kis_color_history.h"
#include "kis_common_colors.h"
#include "kis_color_selector_settings.h"
#include "kis_color_selector_container.h"

#include "kis_action_registry.h"

KisColorSelectorNgDockerWidget::KisColorSelectorNgDockerWidget(QWidget *parent) :
    QWidget(parent),
    m_colorHistoryAction(0),
    m_commonColorsAction(0),
    m_widgetLayout(0),
    m_mainLayout(0),
    m_horizontalPatchesContainer(0),
    m_sidebarLayout(0),
    m_verticalColorPatchesLayout(0),
    m_horizontalColorPatchesLayout(0),
    m_fallbackSettingsButton(new QToolButton(this)),
    m_clearColorHistoryButton(new QToolButton(this)),
    m_canvas(0)
{
    setAutoFillBackground(true);

    m_colorSelectorContainer = new KisColorSelectorContainer(this);
    m_colorHistoryWidget = new KisColorHistory(this);
    m_commonColorsWidget = new KisCommonColors(this);

    //default settings
    //remember to also change the default in the ui file

    //shade selector

    // fallback settings button when the color selector is disabled
    m_fallbackSettingsButton->setIcon(KisIconUtils::loadIcon("configure"));
    m_fallbackSettingsButton->setIconSize(QSize(22,22));
    m_fallbackSettingsButton->setAutoRaise(true);
    m_fallbackSettingsButton->hide();

    //Clear color history button
    m_clearColorHistoryButton->setIcon(KisIconUtils::loadIcon("dialog-cancel"));
    m_clearColorHistoryButton->setIconSize(QSize(22, 22));
    m_clearColorHistoryButton->setAutoRaise(true);

    //layout
    m_widgetLayout = new QHBoxLayout(this);
    m_widgetLayout->setSpacing(0);
    m_widgetLayout->setMargin(0);

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);

    m_horizontalPatchesContainer = new QHBoxLayout();
    m_horizontalPatchesContainer->setSpacing(0);
    m_horizontalPatchesContainer->setMargin(0);

    m_sidebarLayout = new QVBoxLayout();
    m_sidebarLayout->setSpacing(0);
    m_sidebarLayout->setMargin(0);

    m_verticalColorPatchesLayout = new QHBoxLayout();
    m_verticalColorPatchesLayout->setSpacing(0);
    m_verticalColorPatchesLayout->setMargin(0);

    m_horizontalColorPatchesLayout = new QVBoxLayout();
    m_horizontalColorPatchesLayout->setSpacing(0);
    m_horizontalColorPatchesLayout->setMargin(0);

    m_horizontalPatchesContainer->addLayout(m_horizontalColorPatchesLayout);

    m_mainLayout->addWidget(m_colorSelectorContainer);
    m_mainLayout->addLayout(m_horizontalPatchesContainer);

    m_sidebarLayout->addLayout(m_verticalColorPatchesLayout);
    m_sidebarLayout->addWidget(m_clearColorHistoryButton);

    m_widgetLayout->addLayout(m_mainLayout);
    m_widgetLayout->addLayout(m_sidebarLayout);

    updateLayout();

    connect(m_colorSelectorContainer, SIGNAL(openSettings()), this, SLOT(openSettings()));
    connect(m_clearColorHistoryButton, SIGNAL(clicked()), m_colorHistoryWidget, SLOT(clearColorHistory()));

    //emit settingsChanged() if the settings are changed in krita preferences
    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    KisColorSelectorSettingsFactory* factory =
            dynamic_cast<KisColorSelectorSettingsFactory*>(preferenceSetRegistry->get("KisColorSelectorSettingsFactory"));
    Q_ASSERT(factory);
    connect(&(factory->repeater), SIGNAL(settingsUpdated()), this, SIGNAL(settingsChanged()), Qt::UniqueConnection);
    connect(this,     SIGNAL(settingsChanged()), this,                     SLOT(updateLayout()), Qt::UniqueConnection);
    connect(this,     SIGNAL(settingsChanged()), m_commonColorsWidget,     SLOT(updateSettings()), Qt::UniqueConnection);
    connect(this,     SIGNAL(settingsChanged()), m_colorHistoryWidget,     SLOT(updateSettings()), Qt::UniqueConnection);
    connect(this,     SIGNAL(settingsChanged()), m_colorSelectorContainer, SIGNAL(settingsChanged()), Qt::UniqueConnection);
    connect(this,     SIGNAL(settingsChanged()), this,                     SLOT(update()), Qt::UniqueConnection);


    emit settingsChanged();

    m_colorHistoryAction = KisActionRegistry::instance()->makeQAction("show_color_history", this);
    connect(m_colorHistoryAction, SIGNAL(triggered()), m_colorHistoryWidget, SLOT(showPopup()), Qt::UniqueConnection);

    m_commonColorsAction = KisActionRegistry::instance()->makeQAction("show_common_colors", this);
    connect(m_commonColorsAction, SIGNAL(triggered()), m_commonColorsWidget, SLOT(showPopup()), Qt::UniqueConnection);

    connect(m_fallbackSettingsButton, SIGNAL(clicked()), this, SLOT(openSettings()));
}

void KisColorSelectorNgDockerWidget::unsetCanvas()
{
    m_canvas = 0;
    m_commonColorsWidget->unsetCanvas();
    m_colorHistoryWidget->unsetCanvas();
    m_colorSelectorContainer->unsetCanvas();
}

void KisColorSelectorNgDockerWidget::setCanvas(KisCanvas2 *canvas)
{
    if (m_canvas) {
        m_canvas->disconnect(this);
        KActionCollection *ac = m_canvas->viewManager()->actionCollection();
        ac->takeAction(ac->action("show_color_history"));
        ac->takeAction(ac->action("show_common_colors"));
    }

    m_canvas = canvas;

    m_commonColorsWidget->setCanvas(canvas);
    m_colorHistoryWidget->setCanvas(canvas);
    m_colorSelectorContainer->setCanvas(canvas);

    if (m_canvas && m_canvas->viewManager()) {
        KActionCollection* actionCollection = canvas->viewManager()->actionCollection();

        actionCollection->addAction("show_color_history", m_colorHistoryAction);
        actionCollection->addAction("show_common_colors", m_commonColorsAction);

        connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), m_colorSelectorContainer, SLOT(slotUpdateIcons()), Qt::UniqueConnection);
    }

}

void KisColorSelectorNgDockerWidget::openSettings()
{
    if (!m_canvas) return;

    KisColorSelectorSettingsDialog settings;
    if(settings.exec()==QDialog::Accepted) {
        emit settingsChanged();
    }
}


void KisColorSelectorNgDockerWidget::updateLayout()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");

    bool showColorSelector = (bool) cfg.readEntry("showColorSelector", true);

    //color patches
    bool m_lastColorsShow = cfg.readEntry("lastUsedColorsShow", true);
    KisColorPatches::Direction m_lastColorsDirection;
    if(cfg.readEntry("lastUsedColorsAlignment", false))
        m_lastColorsDirection=KisColorPatches::Vertical;
    else
        m_lastColorsDirection=KisColorPatches::Horizontal;

    bool m_commonColorsShow = cfg.readEntry("commonColorsShow", true);
    KisColorPatches::Direction m_commonColorsDirection;
    if(cfg.readEntry("commonColorsAlignment", false))
        m_commonColorsDirection=KisColorPatches::Vertical;
    else
        m_commonColorsDirection=KisColorPatches::Horizontal;

    m_verticalColorPatchesLayout->removeWidget(m_colorHistoryWidget);
    m_verticalColorPatchesLayout->removeWidget(m_commonColorsWidget);
    m_horizontalColorPatchesLayout->removeWidget(m_colorHistoryWidget);
    m_horizontalColorPatchesLayout->removeWidget(m_commonColorsWidget);

    m_sidebarLayout->removeWidget(m_fallbackSettingsButton);
    m_mainLayout->removeWidget(m_fallbackSettingsButton);

    if(m_lastColorsShow==false)
        m_colorHistoryWidget->hide();
    else
        m_colorHistoryWidget->show();

    if(m_commonColorsShow==false) {
        m_commonColorsWidget->hide();
    }
    else {
        m_commonColorsWidget->show();
    }


    bool fallbackSettingsButtonVertical = true;

    if(m_lastColorsShow && m_lastColorsDirection==KisColorPatches::Vertical) {
        m_verticalColorPatchesLayout->addWidget(m_colorHistoryWidget);
    }

    if(m_commonColorsShow && m_commonColorsDirection==KisColorPatches::Vertical) {
        m_verticalColorPatchesLayout->addWidget(m_commonColorsWidget);
    }

    if(m_lastColorsShow && m_lastColorsDirection==KisColorPatches::Horizontal) {
        m_horizontalColorPatchesLayout->addWidget(m_colorHistoryWidget);
        fallbackSettingsButtonVertical = false;
    }

    if(m_commonColorsShow && m_commonColorsDirection==KisColorPatches::Horizontal) {
        m_horizontalColorPatchesLayout->addWidget(m_commonColorsWidget);
        fallbackSettingsButtonVertical = false;
    }

    // prefer the vertical column, if patch components have different layout
    if (m_commonColorsDirection != m_lastColorsDirection) {
        fallbackSettingsButtonVertical = true;
    }

    if (!showColorSelector) {
        if (fallbackSettingsButtonVertical) {
            m_sidebarLayout->addWidget(m_fallbackSettingsButton);
        } else {
            m_horizontalPatchesContainer->addWidget(m_fallbackSettingsButton);
        }

        m_fallbackSettingsButton->show();
    } else {
        m_fallbackSettingsButton->hide();
    }

    updateGeometry();
}
