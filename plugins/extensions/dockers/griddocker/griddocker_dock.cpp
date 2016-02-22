/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "griddocker_dock.h"
//#include "gridwidget.h"

// #include <QLabel>
// #include <QVBoxLayout>
#include <QStatusBar>
#include <klocalizedstring.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include <kis_zoom_manager.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"
#include "grid_config_widget.h"


GridDockerDock::GridDockerDock( )
    : QDockWidget(i18n("Grid"))
    , m_canvas(0)
{
    m_configWidget = new GridConfigWidget(this);
    //m_layout = new QVBoxLayout(page);

    //m_gridWidget = new GridWidget(this);
    //m_gridWidget->setMinimumHeight(50);

    //m_layout->addWidget(m_gridWidget, 1);

    setWidget(m_configWidget);
}

void GridDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    //setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    // if (m_zoomSlider) {
    //     m_layout->removeWidget(m_zoomSlider);
    //     delete m_zoomSlider;
    //     m_zoomSlider = 0;
    // }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    //m_gridWidget->setCanvas(canvas);
    if (m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->zoomController() && m_canvas->viewManager()->zoomController()->zoomAction()) {
        //m_zoomSlider = m_canvas->viewManager()->zoomController()->zoomAction()->createWidget(m_canvas->imageView()->KisView::statusBar());
        //m_layout->addWidget(m_zoomSlider);
    }
}

void GridDockerDock::unsetCanvas()
{
    //setEnabled(false);
    m_canvas = 0;
    //m_gridWidget->unsetCanvas();
}


