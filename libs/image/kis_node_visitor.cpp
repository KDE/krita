/*
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2007, 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_visitor.h"
#include "kis_node.h"

bool KisNodeVisitor::visitAll(KisNode * node, bool breakOnFail)
{
    for (uint i = 0; i < node->childCount(); ++i) {
        if (!node->at(i)->accept(*this)) {
            if (breakOnFail)
                return false;
        }
    }
    return true;
}



/**
 * Visit all child nodes of the given node starting with the last one until one node returns
 * false. Then visitAll returns false, otherwise true.
 *
 * @param node the parent node whose children will be visited
 * @param breakOnFail break if one of the children returns false on accept
 * @return true if none of the childnodes returns false on
 * accepting the visitor.
 */
bool KisNodeVisitor::visitAllInverse(KisNode * node, bool breakOnFail)
{
    KisNodeSP child = node->lastChild();
    while (child) {
        if (!child->accept(*this)) {
            if (breakOnFail)
                return false;
        }
        child = child->prevSibling();
    }
    return true;
}
