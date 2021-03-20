/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "arrangedocker_dock.h"
#include <klocalizedstring.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include "arrange_docker_widget.h"

#include <KoToolProxy.h>
#include <KoShapeManager.h>


ArrangeDockerDock::ArrangeDockerDock( )
    : KDDockWidgets::DockWidget(i18n("Arrange"))
    , m_canvas(0)
{
    m_configWidget = new ArrangeDockerWidget(this);
    m_configWidget->switchState(false);
    setWidget(m_configWidget);
    setEnabled(m_canvas);
}

ArrangeDockerDock::~ArrangeDockerDock()
{
}

void ArrangeDockerDock::setCanvas(KoCanvasBase * canvas)
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

    if (m_canvas) {
        m_canvasConnections.addConnection(
            m_canvas->toolProxy(),
            SIGNAL(toolChanged(QString)),
            this,
            SLOT(slotToolChanged(QString)));

        m_canvasConnections.addConnection(
            m_canvas->shapeManager(),
            SIGNAL(selectionChanged()),
            this,
            SLOT(slotToolChanged()));

        slotToolChanged();
    }
}

void ArrangeDockerDock::unsetCanvas()
{
    setCanvas(0);
}

void ArrangeDockerDock::slotToolChanged()
{
    KActionCollection *collection = m_canvas->viewManager()->actionCollection();
    m_configWidget->setActionCollection(collection);
}

void ArrangeDockerDock::slotToolChanged(QString toolId)
{
    bool enableWidget = (toolId == "InteractionTool") ? true : false;
    m_configWidget->switchState(enableWidget);
    slotToolChanged();
}
