/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_action_manager.h"

#include <QVector>
#include <kactioncollection.h>

#include "kis_action.h"
#include "kis_view2.h"
#include "kis_selection_manager.h"

class KisActionManager::Private {

public:
    Private() {}

    KisView2* view;
    QVector<KisAction*> actions;
};

KisActionManager::KisActionManager(KisView2* view) : d(new Private)
{
    d->view = view;
}

KisActionManager::~KisActionManager()
{
    delete d;
}

void KisActionManager::addAction(const QString& name, KisAction* action, KActionCollection* actionCollection)
{
    actionCollection->addAction(name, action);
    d->actions.append(action);
}

void KisActionManager::updateGUI()
{
    KisNodeSP node = d->view->activeNode();
    KisLayerSP layer = d->view->activeLayer();

    //TODO other flags
    KisAction::ActivationFlags flags;
    if (d->view->activeDevice()) {
        flags |= KisAction::ACTIVE_DEVICE;
    }
    if (node) {
        flags |= KisAction::ACTIVE_NODE;
    }
    if (layer) {
        flags |= KisAction::ACTIVE_LAYER;
        if (layer->inherits("KisShapeLayer")) {
            flags |= KisAction::ACTIVE_SHAPE_LAYER;
        }
    }
    KisSelectionManager* selectionManager = d->view->selectionManager();
    if (selectionManager->havePixelsSelected()) {
        flags |= KisAction::PIXELS_SELECTED;
    }
    if (selectionManager->haveShapesSelected()) {
        flags |= KisAction::SHAPES_SELECTED;
    }
    if (selectionManager->havePixelSelectionWithPixels()) {
        flags |= KisAction::PIXEL_SELECTION_WITH_PIXELS;
    }
    if (selectionManager->havePixelsInClipboard()) {
        flags |= KisAction::PIXELS_IN_CLIPBOARD;
    }
    if (selectionManager->haveShapesInClipboard()) {
        flags |= KisAction::SHAPES_IN_CLIPBOARD;
    }

    KisAction::ActivationConditions conditions = KisAction::NO_CONDITION;
    if (node && node->isEditable()) {
        conditions |= KisAction::ACTIVE_NODE_EDITABLE;
    }
    if (d->view->selectionEditable()) {
        conditions |= KisAction::SELECTION_EDITABLE;
    }

    foreach(KisAction* action, d->actions) {
        bool enable = action->activationFlags() & flags;
        enable = enable && (int)(action->activationConditions() & conditions) == (int)action->activationConditions();
        action->setEnabled(enable);
    }
}
