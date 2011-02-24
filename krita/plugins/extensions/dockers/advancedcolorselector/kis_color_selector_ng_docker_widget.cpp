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

#include "kis_color_selector_ng_docker_widget.h"
#include "ui_wdg_color_selector_settings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>
#include <KAction>
#include <KActionCollection>

#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_layer_manager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_color_space_selector.h"
#include "kis_preference_set_registry.h"
#include "kis_node.h"
#include "kis_paint_device.h"

#include "kis_color_history.h"
#include "kis_common_colors.h"
#include "kis_color_selector_settings.h"
#include "kis_color_selector_container.h"

#include <KDebug>

KisColorSelectorNgDockerWidget::KisColorSelectorNgDockerWidget(QWidget *parent) :
    QWidget(parent),
    m_colorHistoryAction(0),
    m_commonColorsAction(0),
    m_verticalColorPatchesLayout(0),
    m_horizontalColorPatchesLayout(0),
    m_canvas(0)
{
    setAutoFillBackground(true);

    m_colorSelectorContainer = new KisColorSelectorContainer(this);
    m_colorHistoryWidget = new KisColorHistory(this);
    m_commonColorsWidget = new KisCommonColors(this);

    //default settings
    //remember to also change the default in the ui file

    //shade selector

    //layout
    m_verticalColorPatchesLayout = new QHBoxLayout();
    m_verticalColorPatchesLayout->setSpacing(0);
    m_verticalColorPatchesLayout->setMargin(0);
    m_verticalColorPatchesLayout->addWidget(m_colorSelectorContainer);

    m_horizontalColorPatchesLayout = new QVBoxLayout(this);
    m_horizontalColorPatchesLayout->setSpacing(0);
    m_horizontalColorPatchesLayout->setMargin(0);
    m_horizontalColorPatchesLayout->addLayout(m_verticalColorPatchesLayout);

    updateLayout();

    connect(m_colorSelectorContainer, SIGNAL(openSettings()), this, SLOT(openSettings()));

    //emit settingsChanged() if the settings are changed in krita preferences
    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    KisColorSelectorSettings* settings = dynamic_cast<KisColorSelectorSettings*>(preferenceSetRegistry->get("advancedColorSelector"));
    Q_ASSERT(settings);

    connect(settings, SIGNAL(settingsChanged()), this,                     SIGNAL(settingsChanged()));
    connect(this,     SIGNAL(settingsChanged()), this,                     SLOT(updateLayout()));
    connect(this,     SIGNAL(settingsChanged()), m_commonColorsWidget,     SLOT(updateSettings()));
    connect(this,     SIGNAL(settingsChanged()), m_colorHistoryWidget,     SLOT(updateSettings()));
    connect(this,     SIGNAL(settingsChanged()), m_colorSelectorContainer, SIGNAL(settingsChanged()));
    connect(this,     SIGNAL(settingsChanged()), this,                     SLOT(update()));

    emit settingsChanged();
}

void KisColorSelectorNgDockerWidget::setCanvas(KisCanvas2 *canvas)
{
    Q_ASSERT(canvas);
    m_commonColorsWidget->setCanvas(canvas);
    m_colorHistoryWidget->setCanvas(canvas);
    m_colorSelectorContainer->setCanvas(canvas);
    m_canvas = canvas;

    if(m_canvas->view()->layerManager())
        connect(m_canvas->view()->layerManager(), SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(reactOnLayerChange()));

    KActionCollection* actionCollection = canvas->view()->actionCollection();

    if(m_colorHistoryAction!=0)
        return;     //we don't need to create the actions a second time

    m_colorHistoryAction = new KAction("Show color history", this);
    m_colorHistoryAction->setShortcut(QKeySequence(tr("H")));
    connect(m_colorHistoryAction, SIGNAL(triggered()), m_colorHistoryWidget, SLOT(showPopup()));
    actionCollection->addAction("show_color_history", m_colorHistoryAction);

    m_commonColorsAction = new KAction("Show common colors", this);
    m_commonColorsAction->setShortcut(QKeySequence(tr("C")));
    connect(m_commonColorsAction, SIGNAL(triggered()), m_commonColorsWidget, SLOT(showPopup()));
    actionCollection->addAction("show_common_colors", m_commonColorsAction);
}

void KisColorSelectorNgDockerWidget::openSettings()
{
    Q_ASSERT(m_canvas);

    KisColorSelectorSettingsDialog settings;
    if(settings.exec()==QDialog::Accepted) {
        emit settingsChanged();
    }
}


void KisColorSelectorNgDockerWidget::updateLayout()
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");


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

    if(m_lastColorsShow && m_lastColorsDirection==KisColorPatches::Vertical) {
        m_verticalColorPatchesLayout->addWidget(m_colorHistoryWidget);
    }

    if(m_commonColorsShow && m_commonColorsDirection==KisColorPatches::Vertical) {
        m_verticalColorPatchesLayout->addWidget(m_commonColorsWidget);
    }

    if(m_lastColorsShow && m_lastColorsDirection==KisColorPatches::Horizontal) {
        m_horizontalColorPatchesLayout->addWidget(m_colorHistoryWidget);
    }

    if(m_commonColorsShow && m_commonColorsDirection==KisColorPatches::Horizontal) {
        m_horizontalColorPatchesLayout->addWidget(m_commonColorsWidget);
    }

    updateGeometry();
}

void KisColorSelectorNgDockerWidget::reactOnLayerChange()
{
    KisNodeSP node = m_canvas->view()->resourceProvider()->currentNode();
    if (node) {
        KisPaintDeviceSP device = node->paintDevice();
        if (device) {
            m_colorHistoryAction->setEnabled(true);
            m_commonColorsAction->setEnabled(true);
        }
        else {
            m_colorHistoryAction->setEnabled(false);
            m_commonColorsAction->setEnabled(false);
        }
    }
}
