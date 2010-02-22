/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_merge_walker.h"


KisMergeWalker::KisMergeWalker(QRect cropRect)
    : KisBaseRectsWalker(cropRect)
{
}

KisMergeWalker::~KisMergeWalker()
{
}

void KisMergeWalker::startTrip(KisNodeSP startWith)
{
    visitHigherNode(startWith, N_FILTHY);

    KisNodeSP prevNode = startWith->prevSibling();
    if(prevNode)
        visitLowerNode(prevNode);
}

void KisMergeWalker::visitHigherNode(KisNodeSP node, NodePosition positionToFilthy)
{
    KisNodeSP nextNode = node->nextSibling();
    KisNodeSP prevNode = node->prevSibling();

    registerChangeRect(node);

    if (nextNode)
        visitHigherNode(nextNode, N_ABOVE_FILTHY);
    else if (node->parent())
        startTrip(node->parent());

    positionToFilthy |=
        !nextNode ? N_TOPMOST : !prevNode ? N_BOTTOMMOST : N_NORMAL;
    registerNeedRect(node, positionToFilthy);
}

void KisMergeWalker::visitLowerNode(KisNodeSP node)
{
    KisNodeSP prevNode = node->prevSibling();
    NodePosition position =
        N_BELOW_FILTHY | (prevNode ? N_NORMAL : N_BOTTOMMOST);
    registerNeedRect(node, position);

    if (prevNode)
        visitLowerNode(prevNode);
}
