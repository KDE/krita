/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_animation_curve_docker.h"
#include "kis_animation_curves_model.h"
#include "kis_animation_curves_view.h"

#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_shape_controller.h"
#include "kis_signal_auto_connection.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "klocalizedstring.h"

struct KisAnimationCurveDocker::Private
{
    Private(QWidget *parent)
        : model(new KisAnimationCurvesModel(parent)),
          view(new KisAnimationCurvesView(parent))
    {
        view->setModel(model);
    }

    KisAnimationCurvesModel *model;
    KisAnimationCurvesView *view;

    QPointer<KisCanvas2> canvas;

    KisSignalAutoConnectionsStore canvasConnections;
};

KisAnimationCurveDocker::KisAnimationCurveDocker()
    : QDockWidget(i18n("Animation curves"))
    , m_d(new Private(this))
{
    setWidget(m_d->view);
}

KisAnimationCurveDocker::~KisAnimationCurveDocker()
{}

void KisAnimationCurveDocker::setCanvas(KoCanvasBase * canvas)
{
    if (canvas && m_d->canvas == canvas) return;

    if (m_d->canvas) {
        m_d->canvasConnections.clear();
        m_d->canvas->disconnectCanvasObserver(this);
    }

    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if(m_d->canvas) {
        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->model, SLOT(slotCurrentNodeChanged(KisNodeSP))
        );

        m_d->model->slotCurrentNodeChanged(m_d->canvas->viewManager()->activeNode());
    }
}

void KisAnimationCurveDocker::unsetCanvas()
{
    setCanvas(0);
}

void KisAnimationCurveDocker::setMainWindow(KisViewManager *kisview)
{

}
