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
{
    setCropRect(cropRect);
}

KisMergeWalker::~KisMergeWalker()
{
}

KisBaseRectsWalker::UpdateType KisMergeWalker::type() const
{
    return KisBaseRectsWalker::UPDATE;
}

void KisMergeWalker::startTrip(KisNodeSP startWith)
{
    if(isMask(startWith)) {
        startTripWithMask(startWith);
        return;
    }

    visitHigherNode(startWith, N_FILTHY);

    KisNodeSP prevNode = startWith->prevSibling();
    if(prevNode)
        visitLowerNode(prevNode);
}

void KisMergeWalker::startTripWithMask(KisNodeSP filthyMask)
{
    adjustMasksChangeRect(filthyMask);

    KisNodeSP parentLayer = filthyMask->parent();
    Q_ASSERT(parentLayer);

    KisNodeSP nextNode = parentLayer->nextSibling();
    KisNodeSP prevNode = parentLayer->prevSibling();

    if (nextNode)
        visitHigherNode(nextNode, N_ABOVE_FILTHY);
    else if (parentLayer->parent())
        startTrip(parentLayer->parent());

    NodePosition positionToFilthy = N_FILTHY_PROJECTION |
        (!nextNode ? N_TOPMOST : !prevNode ? N_BOTTOMMOST : N_NORMAL);
    registerNeedRect(parentLayer, positionToFilthy);

    if(prevNode)
        visitLowerNode(prevNode);
}

void KisMergeWalker::visitHigherNode(KisNodeSP node, NodePosition positionToFilthy)
{
    KisNodeSP nextNode = node->nextSibling();
    KisNodeSP prevNode = node->prevSibling();
    positionToFilthy |=
        !nextNode ? N_TOPMOST : !prevNode ? N_BOTTOMMOST : N_NORMAL;

    registerChangeRect(node, positionToFilthy);

    if (nextNode)
        visitHigherNode(nextNode, N_ABOVE_FILTHY);
    else if (node->parent())
        startTrip(node->parent());

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
