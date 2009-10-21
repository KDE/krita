/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007,2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_node_visitor.h"

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
