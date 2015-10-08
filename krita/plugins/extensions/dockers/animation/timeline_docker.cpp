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

TimelineDocker::TimelineDocker()
    : QDockWidget(i18n("Timeline"))
    , m_canvas(0)
    , m_timelineWidget(new TimelineWidget(this))
{
    setWidget(m_timelineWidget);

    m_model = new KisTimelineModel(this);
    m_timelineWidget->setModel(m_model);

    m_toggleOnionSkinAction = new KisAction(i18n("Toggle onion skin"), this);
    m_toggleOnionSkinAction->setActivationFlags(KisAction::ACTIVE_LAYER);

    connect(m_toggleOnionSkinAction, SIGNAL(triggered()), this, SLOT(toggleOnionSkin()));
}

void TimelineDocker::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas == canvas) return;

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_model->setDummiesFacade(0, 0, 0);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(canvas != 0);
    m_timelineWidget->setCanvas(m_canvas);

    if(m_canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        KisDummiesFacadeBase *kritaDummiesFacade = static_cast<KisDummiesFacadeBase*>(kritaShapeController);
        m_model->setDummiesFacade(kritaDummiesFacade, m_canvas->image(), kritaShapeController);
    }
}

void TimelineDocker::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
    m_model->setDummiesFacade(0, 0, 0);
    m_timelineWidget->setCanvas(0);
}

void TimelineDocker::setMainWindow(KisViewManager *view)
{
    KisActionManager *actionManager = view->actionManager();
    actionManager->addAction("toggle_onion_skin", m_toggleOnionSkinAction);
}

void TimelineDocker::toggleOnionSkin()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer *layer = qobject_cast<KisPaintLayer*>(node.data());
        layer->setOnionSkinEnabled(!layer->onionSkinEnabled());
        m_canvas->image()->refreshGraphAsync();
    }
}

#include "timeline_docker.moc"
