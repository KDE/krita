/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "timeline_docker.h"

#include "kis_canvas2.h"
#include "kis_image.h"
#include <KoIcon.h>
#include "KisViewManager.h"
#include "kis_paint_layer.h"
#include "KisDocument.h"
#include "kis_dummies_facade.h"
#include "kis_shape_controller.h"
#include "kis_action.h"
#include "kis_action_manager.h"

#include "timeline_frames_model.h"
#include "timeline_frames_view.h"
#include "kis_animation_frame_cache.h"
#include "kis_signal_auto_connection.h"
#include "kis_node_manager.h"

#include <QPointer>

struct TimelineDocker::Private
{
    Private(QWidget *parent)
        : model(new TimelineFramesModel(parent)),
          view(new TimelineFramesView(parent))
    {
        view->setModel(model);
    }

    TimelineFramesModel *model;
    TimelineFramesView *view;

    QPointer<KisCanvas2> canvas;

    KisSignalAutoConnectionsStore canvasConnections;
};

TimelineDocker::TimelineDocker()
    : QDockWidget(i18n("Timeline"))
    , m_d(new Private(this))
{
    setWidget(m_d->view);
}

TimelineDocker::~TimelineDocker()
{
}

struct NodeManagerInterface : TimelineFramesModel::NodeManipulationInterface
{
    NodeManagerInterface(KisNodeManager *manager) : m_manager(manager) {}

    KisLayerSP addPaintLayer() const {
        return m_manager->createPaintLayer();
    }

    void removeNode(KisNodeSP node) const {
        m_manager->removeSingleNode(node);
    }

private:
    KisNodeManager *m_manager;
};

void TimelineDocker::setCanvas(KoCanvasBase * canvas)
{
    if (m_d->canvas == canvas) return;

    if (m_d->canvas) {
        m_d->canvasConnections.clear();
        m_d->model->setDummiesFacade(0, 0);
        m_d->model->setFrameCache(0);
        m_d->model->setAnimationPlayer(0);
        m_d->model->setNodeManipulationInterface(0);
        m_d->canvas->disconnectCanvasObserver(this);
    }

    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if(m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->model->setDummiesFacade(kritaShapeController, m_d->canvas->image());
        m_d->model->setFrameCache(m_d->canvas->frameCache());
        m_d->model->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->model->setNodeManipulationInterface(
            new NodeManagerInterface(m_d->canvas->viewManager()->nodeManager()));

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->model, SLOT(slotCurrentNodeChanged(KisNodeSP)));

        m_d->canvasConnections.addConnection(
            m_d->model, SIGNAL(requestCurrentNodeChanged(KisNodeSP)),
            m_d->canvas->viewManager()->nodeManager(), SLOT(slotNonUiActivatedNode(KisNodeSP)));

        m_d->model->slotCurrentNodeChanged(m_d->canvas->viewManager()->activeNode());
    }
}

void TimelineDocker::unsetCanvas()
{
    setCanvas(0);
}

void TimelineDocker::setMainWindow(KisViewManager *view)
{
    Q_UNUSED(view);
    //KisActionManager *actionManager = view->actionManager();
    //actionManager->addAction("toggle_onion_skin", m_toggleOnionSkinAction);
}

#include "timeline_docker.moc"
