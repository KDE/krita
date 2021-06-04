/*
 *  SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisNodeActivationActionCreatorVisitor.h"

#include <kis_node_manager.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <QObject>
#include <QRect>
#include <kis_projection_leaf.h>

KisNodeActivationActionCreatorVisitor::KisNodeActivationActionCreatorVisitor(KActionCollection *actionCollection, KisNodeManager *nodeManager)
    : m_nodeManager(nodeManager)
    , m_actionCollection(actionCollection)
{
}


bool KisNodeActivationActionCreatorVisitor::createAction(KisNode *node)
{
    if (!node->projectionLeaf()->isRoot()) {
        QAction *action = new QAction(i18nc("A temporary action that actives a layer or mask",
                                            "Activate %1", node->name()),
                                      m_actionCollection);
        action->setObjectName(QString("select_%1").arg(node->name()));
        action->setProperty("node", node->name());
        action->setIcon(node->icon());
        QObject::connect(action, SIGNAL(triggered()), m_nodeManager, SLOT(slotUiActivateNode()));
        m_actionCollection->addAction(action->objectName(), action);
    }

    visitAll(node);

    return true;
}
