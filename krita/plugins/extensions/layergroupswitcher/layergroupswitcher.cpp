/*
 * layergroupswitcher.cpp -- Part of Krita
 *
 * Copyright (c) 2013 Boudewijn Rempt (boud@valdyas.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "layergroupswitcher.h"

#include <klocale.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_node.h>
#include <kis_node_manager.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view2.h>

#include "kis_action.h"

K_PLUGIN_FACTORY(LayerGroupSwitcherFactory, registerPlugin<LayerGroupSwitcher>();)
K_EXPORT_PLUGIN(LayerGroupSwitcherFactory("krita"))

LayerGroupSwitcher::LayerGroupSwitcher(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent, "")
{
    KisAction *action  = new KisAction(i18n("Move into previous group"), this);
    addAction("LayerGroupSwitcher/previous", action);
    connect(action, SIGNAL(triggered()), this, SLOT(moveIntoPreviousGroup()));

    action = new KisAction(i18n("Move into next group"), this);
    addAction("LayerGroupSwitcher/next", action);
    connect(action, SIGNAL(triggered()), this, SLOT(moveIntoNextGroup()));

}

LayerGroupSwitcher::~LayerGroupSwitcher()
{
}

void LayerGroupSwitcher::moveIntoNextGroup()
{
    KisImageWSP image = m_view->image();
    KisNodeManager *nodeManager = m_view->nodeManager();
    KisLayerSP active = nodeManager->activeLayer();
    if (!active) {
        return;
    }
    if (active->parentLayer().data() == image->rootLayer().data()) {
        active->setVisible(false);
        nodeManager->activateNextNode();
        active = nodeManager->activeLayer();
        if (active) {
            active->setVisible(true);
        }
    }
    else if (active->parent()) {
        KisNodeSP parent = active->parent();
        if (parent) {
            int indexInGroup = active->parent()->index(active);
            nodeManager->slotNonUiActivatedNode(parent);
            parent->setVisible(false);
            nodeManager->activateNextNode();
            active = nodeManager->activeLayer();
            if (active) {
                active->setVisible(true);
            }
            KisNodeSP child = active->at(indexInGroup);
            if (child) {
                nodeManager->slotNonUiActivatedNode(child);
                child->setVisible(true);
            }
        }
    }
    image->refreshGraph();

}

void LayerGroupSwitcher::moveIntoPreviousGroup()
{
    KisImageWSP image = m_view->image();
    KisNodeManager *nodeManager = m_view->nodeManager();
    KisLayerSP active = nodeManager->activeLayer();
    if (!active) {
        return;
    }
    if (active->parentLayer().data() == image->rootLayer().data()) {
        active->setVisible(false);
        nodeManager->activatePreviousNode();
        active = nodeManager->activeLayer();
        if (active) {
            active->setVisible(true);
        }
    }
    else if (active->parent()) {
        KisNodeSP parent = active->parent();
        if (parent) {
            int indexInGroup = active->parent()->index(active);
            nodeManager->slotNonUiActivatedNode(parent);
            parent->setVisible(false);
            nodeManager->activatePreviousNode();
            active = nodeManager->activeLayer();
            if (active) {
                active->setVisible(true);
            }
            KisNodeSP child = active->at(indexInGroup);
            if (child) {
                nodeManager->slotNonUiActivatedNode(child);
                child->setVisible(true);
            }
        }
    }
    image->refreshGraph();
}
