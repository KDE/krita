/*
 * layergroupswitcher.cpp -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2013 Boudewijn Rempt (boud@valdyas.org)
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "layergroupswitcher.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_node.h>
#include <kis_node_manager.h>
#include <kis_global.h>
#include <kis_types.h>
#include <KisViewManager.h>

#include "kis_action.h"

K_PLUGIN_FACTORY_WITH_JSON(LayerGroupSwitcherFactory, "kritalayergroupswitcher.json", registerPlugin<LayerGroupSwitcher>();)

LayerGroupSwitcher::LayerGroupSwitcher(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
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
    KisImageSP image = viewManager()->image().toStrongRef();
    if (!image) {
        return;
    }
    KisNodeManager *nodeManager = viewManager()->nodeManager();
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
    KisImageSP image = viewManager()->image().toStrongRef();
    if (!image) {
        return;
    }
    KisNodeManager *nodeManager = viewManager()->nodeManager();
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

#include "layergroupswitcher.moc"
