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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kaction.h>
#include <kactioncollection.h>

#include "kis_canvas2.h"
#include "kis_view2.h"
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
        KActionCollection *ac = m_canvas->view()->actionCollection();
        ac->takeAction(ac->action("show_color_history"));
        ac->takeAction(ac->action("show_common_colors"));
    }

    m_canvas = canvas;
    Q_ASSERT(canvas);
    m_commonColorsWidget->setCanvas(canvas);
    m_colorHistoryWidget->setCanvas(canvas);
    m_colorSelectorContainer->setCanvas(canvas);


    if (m_canvas->view()->nodeManager()) {
        connect(m_canvas->view()->nodeManager(), SIGNAL(sigLayerActivated(KisLayerSP)), SLOT(reactOnLayerChange()), Qt::UniqueConnection);
    }
    KActionCollection* actionCollection = canvas->view()->actionCollection();

    if (!m_colorHistoryAction) {
        m_colorHistoryAction = new KAction("Show color history", this);
        m_colorHistoryAction->setShortcut(QKeySequence(tr("H")));
        connect(m_colorHistoryAction, SIGNAL(triggered()), m_colorHistoryWidget, SLOT(showPopup()), Qt::UniqueConnection);
    }
    actionCollection->addAction("show_color_history", m_colorHistoryAction);

    if (!m_commonColorsAction) {
        m_commonColorsAction = new KAction("Show common colors", this);
        m_commonColorsAction->setShortcut(QKeySequence(tr("C")));
        connect(m_commonColorsAction, SIGNAL(triggered()), m_commonColorsWidget, SLOT(showPopup()), Qt::UniqueConnection);
    }
    actionCollection->addAction("show_common_colors", m_commonColorsAction);


    reactOnLayerChange();
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
    // this will trigger settings update and therefore an update of the color space setting and therefore it will change
    // the color space to the current layer
    emit settingsChanged();
    if (m_canvas) {
        KisNodeSP node = m_canvas->view()->resourceProvider()->currentNode();
        if (node && node->paintDevice()) {
            KisPaintDeviceSP device = node->paintDevice();
            connect(device.data(), SIGNAL(profileChanged(const KoColorProfile*)), this, SIGNAL(settingsChanged()), Qt::UniqueConnection);
            connect(device.data(), SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SIGNAL(settingsChanged()), Qt::UniqueConnection);
            
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
}
