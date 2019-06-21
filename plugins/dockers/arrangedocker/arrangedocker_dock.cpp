/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#include "arrangedocker_dock.h"
#include <klocalizedstring.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include "arrange_docker_widget.h"

#include <KoToolProxy.h>
#include <KoShapeManager.h>


ArrangeDockerDock::ArrangeDockerDock( )
    : QDockWidget(i18n("Arrange"))
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
