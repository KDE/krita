/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "overviewdocker_dock.h"
#include "overviewwidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStatusBar>
#include <kis_slider_spin_box.h>
#include <klocalizedstring.h>

#include "kis_canvas2.h"
#include <KisViewManager.h>
#include <kactioncollection.h>
#include <kis_action.h>
#include <kis_zoom_manager.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"
#include "kis_canvas_controller.h"
#include "kis_icon_utils.h"

OverviewDockerDock::OverviewDockerDock( )
    : QDockWidget(i18n("Overview"))
    , m_zoomSlider(nullptr)
    , m_rotateSlider(nullptr)
    , m_mirrorCanvas(nullptr)
    , m_canvas(nullptr)
{
    QWidget *page = new QWidget(this);
    m_layout = new QVBoxLayout(page);
    m_horizontalLayout = new QHBoxLayout();

    m_overviewWidget = new OverviewWidget(this);
    m_overviewWidget->setMinimumHeight(50);
    m_overviewWidget->setBackgroundRole(QPalette::AlternateBase);
    m_overviewWidget->setAutoFillBackground(true); // paints background role before paint()

    m_layout->addWidget(m_overviewWidget, 1);

    setWidget(page);
}

void OverviewDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != nullptr);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    if (m_zoomSlider) {
        m_layout->removeWidget(m_zoomSlider);
        delete m_zoomSlider;
        m_zoomSlider = nullptr;
    }

    if (m_rotateSlider) {
        m_horizontalLayout->removeWidget(m_rotateSlider);
        delete m_rotateSlider;
        m_rotateSlider = nullptr;
    }

    if (m_mirrorCanvas) {
        m_horizontalLayout->removeWidget(m_mirrorCanvas);
        delete m_mirrorCanvas;
        m_mirrorCanvas = nullptr;
    }

    m_layout->removeItem(m_horizontalLayout);

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    m_overviewWidget->setCanvas(canvas);
    if (m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->zoomController() && m_canvas->viewManager()->zoomController()->zoomAction()) {
        m_zoomSlider = m_canvas->viewManager()->zoomController()->zoomAction()->createWidget(m_canvas->imageView()->KisView::statusBar());
        m_layout->addWidget(m_zoomSlider);

        m_rotateSlider = new KisDoubleSliderSpinBox();
        m_rotateSlider->setRange(-180, 180, 2);
        m_rotateSlider->setValue(m_canvas->rotationAngle());
        m_rotateSlider->setPrefix(i18n("Rotation: "));
        m_rotateSlider->setSuffix("°");
        connect(m_rotateSlider, SIGNAL(valueChanged(qreal)), this, SLOT(rotateCanvasView(qreal)), Qt::UniqueConnection);
        connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)), this, SLOT(updateSlider()));

        m_mirrorCanvas = new QToolButton();
        QList<QAction *> actions = m_canvas->viewManager()->actionCollection()->actions();
        Q_FOREACH(QAction* action, actions) {
            if (action->objectName()=="mirror_canvas") {
                m_mirrorCanvas->setDefaultAction(action);
            }
        }
        m_horizontalLayout->addWidget(m_mirrorCanvas);
        m_horizontalLayout->addWidget(m_rotateSlider);
        m_layout->addLayout(m_horizontalLayout);
    }
}

void OverviewDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = nullptr;
    m_overviewWidget->unsetCanvas();
}

void OverviewDockerDock::rotateCanvasView(qreal rotation)
{
    KisCanvasController *canvasController =
            dynamic_cast<KisCanvasController*>(m_canvas->viewManager()->canvasBase()->canvasController());
    if (canvasController) {
        canvasController->rotateCanvas(rotation-m_canvas->rotationAngle());
    }
}

void OverviewDockerDock::updateSlider()
{
    qreal rotation = m_canvas->rotationAngle();
    if (rotation > 180) {
        rotation = rotation - 360;
    } else if (rotation < -180) {
        rotation = rotation + 360;
    }
    if (m_rotateSlider->value() != rotation) {
        m_rotateSlider->setValue(rotation);
    }
}


