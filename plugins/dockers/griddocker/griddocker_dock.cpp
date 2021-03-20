/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "griddocker_dock.h"
//#include "gridwidget.h"

// #include <QLabel>
// #include <QVBoxLayout>
#include <QStatusBar>
#include <klocalizedstring.h>

#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_zoom_manager.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_signal_compressor.h>
#include <kis_grid_manager.h>
#include <kis_grid_config.h>
#include <kis_guides_manager.h>
#include <kis_guides_config.h>
#include <kis_action.h>
#include <KisDocument.h>

#include "grid_config_widget.h"


GridDockerDock::GridDockerDock( )
    : QDockWidget(i18n("Grid and Guides"))
    , m_canvas(0)
{
    m_configWidget = new GridConfigWidget(this);
    connect(m_configWidget, SIGNAL(gridValueChanged()), SLOT(slotGuiGridConfigChanged()));
    connect(m_configWidget, SIGNAL(guidesValueChanged()), SLOT(slotGuiGuidesConfigChanged()));
    setWidget(m_configWidget);
    setEnabled(m_canvas);
}

GridDockerDock::~GridDockerDock()
{
}

void GridDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(canvas && m_canvas == canvas)
        return;

    if (m_canvas) {
        m_canvasConnections.clear();
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    m_canvas = canvas ? dynamic_cast<KisCanvas2*>(canvas) : 0;
    setEnabled(m_canvas);

    if (m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->document()) {
        m_canvasConnections.addConnection(
            m_canvas->viewManager()->gridManager(),
            SIGNAL(sigRequestUpdateGridConfig(KisGridConfig)),
            this,
            SLOT(slotGridConfigUpdateRequested(KisGridConfig)));

        slotGridConfigUpdateRequested(m_canvas->viewManager()->document()->gridConfig());

        KisAction* action = m_canvas->viewManager()->actionManager()->actionByName("view_ruler");

        m_canvasConnections.addConnection(m_configWidget,SIGNAL(showRulersChanged(bool)),action,SLOT(setChecked(bool)));
        m_canvasConnections.addConnection(action,SIGNAL(toggled(bool)),m_configWidget,SLOT(setShowRulers(bool)));
        m_configWidget->setShowRulers(action->isChecked());

        m_canvasConnections.addConnection(
            m_canvas->viewManager()->guidesManager(),
            SIGNAL(sigRequestUpdateGuidesConfig(KisGuidesConfig)),
            this,
            SLOT(slotGuidesConfigUpdateRequested(KisGuidesConfig)));
        slotGuidesConfigUpdateRequested(m_canvas->viewManager()->document()->guidesConfig());

        // isometric grid only available with OpenGL
        if (m_canvas->canvasIsOpenGL()) {
            m_configWidget->enableIsometricGrid(true);
        } else {
            m_configWidget->enableIsometricGrid(false);
        }

    }
}

void GridDockerDock::unsetCanvas()
{
    setCanvas(0);
}

void GridDockerDock::slotGuiGridConfigChanged()
{
    if (!m_canvas) return;
    m_canvas->viewManager()->gridManager()->setGridConfig(m_configWidget->gridConfig());
}

void GridDockerDock::slotGridConfigUpdateRequested(const KisGridConfig &config)
{
    m_configWidget->setGridConfig(config);
}

void GridDockerDock::slotGuiGuidesConfigChanged()
{
    if (!m_canvas) return;
    m_canvas->viewManager()->guidesManager()->setGuidesConfig(m_configWidget->guidesConfig());
}

void GridDockerDock::slotGuidesConfigUpdateRequested(const KisGuidesConfig &config)
{
    m_configWidget->setGuidesConfig(config);
}
