/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "overviewdocker_dock.h"
#include "overviewwidget.h"

#include <QLabel>
#include <QVBoxLayout>

#include <klocale.h>
#include <kstatusbar.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include <kis_zoom_manager.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"


OverviewDockerDock::OverviewDockerDock( )
    : QDockWidget(i18n("Overview"))
    , m_zoomSlider(0)
    , m_canvas(0)
{
    QWidget *page = new QWidget(this);
    m_layout = new QVBoxLayout(page);

    m_overviewWidget = new OverviewWidget(this);
    m_overviewWidget->setMinimumHeight(50);

    m_layout->addWidget(m_overviewWidget, 1);

    setWidget(page);
}

void OverviewDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }
    if (m_zoomSlider) {
        m_layout->removeWidget(m_zoomSlider);
        delete m_zoomSlider;
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    KIS_ASSERT_RECOVER_RETURN(m_canvas);

    m_overviewWidget->setCanvas(canvas);
    m_zoomSlider = m_canvas->viewManager()->zoomController()->zoomAction()->createWidget(m_canvas->imageView()->KisView::statusBar());
    m_layout->addWidget(m_zoomSlider);
}

void OverviewDockerDock::unsetCanvas()
{
    m_canvas = 0;
    m_overviewWidget->unsetCanvas();
}


#include "overviewdocker_dock.moc"
