/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_REFRESH_SUBTREE_WALKER_H
#define __KIS_REFRESH_SUBTREE_WALKER_H

#include "kis_node.h"
#include "kis_types.h"
#include "kis_clone_layer.h"
#include "kis_base_rects_walker.h"


class KRITAIMAGE_EXPORT KisRefreshSubtreeWalker : public virtual KisBaseRectsWalker
{

public:
    KisRefreshSubtreeWalker(QRect cropRect)
    {
        setCropRect(cropRect);
    }

    virtual ~KisRefreshSubtreeWalker()
    {
    }

private:
    inline bool canHaveChildLayers(KisNodeSP node) {
        return qobject_cast<KisGroupLayer*>(node.data());
    }
    static inline bool isRootNode(KisNodeSP node) {
        return !node->parent();
    }

protected:
    KisRefreshSubtreeWalker() {}


    QRect calculateChangeRect(KisNodeSP startWith,
                              const QRect &requestedRect,
                              bool *changeRectVaries) {

        if(!isLayer(startWith))
            return requestedRect;

        QRect childrenRect;
        QRect tempRect = requestedRect;

        KisNodeSP currentNode = startWith->firstChild();
        KisNodeSP prevNode;
        KisNodeSP nextNode;

        while(currentNode) {
            nextNode = currentNode->nextSibling();

            if(isLayer(currentNode)) {
                tempRect = calculateChangeRect(currentNode, tempRect, changeRectVaries);

                if(!*changeRectVaries)
                    *changeRectVaries = tempRect != requestedRect;

                childrenRect = tempRect;
                prevNode = currentNode;
            }

            currentNode = nextNode;
        }

        tempRect = startWith->changeRect(requestedRect | childrenRect);

        if(!*changeRectVaries)
            *changeRectVaries = tempRect != requestedRect;

        return tempRect;
    }

    void startTrip(KisNodeSP startWith) {

        bool changeRectVaries = false;
        QRect changeRect = calculateChangeRect(startWith, requestedRect(), &changeRectVaries);
        setExplicitChangeRect(changeRect, changeRectVaries);

        if(startWith == startNode()) {
            NodePosition pos = N_FILTHY;
            if(!startWith->nextSibling()) pos |= N_TOPMOST;
            if(!startWith->prevSibling()) pos |= N_BOTTOMMOST;
            registerNeedRect(startWith, N_TOPMOST | N_FILTHY);
        }


        KisNodeSP currentNode = startWith->lastChild();
        if(!currentNode) return;

        registerNeedRect(currentNode, N_TOPMOST | N_FILTHY);

        KisNodeSP prevNode = currentNode->prevSibling();
        while ((currentNode = prevNode)) {
            prevNode = currentNode->prevSibling();
            registerNeedRect(currentNode,
                             (!prevNode ? N_BOTTOMMOST : N_NORMAL) | N_FILTHY);
        }

        currentNode = startWith->lastChild();
        do {
            if(canHaveChildLayers(currentNode))
                startTrip(currentNode);
        } while ((currentNode = currentNode->prevSibling()));
    }

    void registerNeedRect(KisNodeSP node, NodePosition position) {
        KisBaseRectsWalker::registerNeedRect(node, position);

        KisCloneLayerSP cloneLayer = qobject_cast<KisCloneLayer*>(node.data());
        if(cloneLayer) {
            /**
             * We need to check whether the source of the clone is going
             * to be updated after the clone itself. If so we need to
             * pre-update it manually. This can be checked by the precedence
             * of the source in the nodes stack
             */
            if(isRegistered(cloneLayer->copyFrom())) {
                registerNeedRect(cloneLayer->copyFrom(), N_EXTRA | N_FILTHY);
            }
        }
    }

    bool isRegistered(KisNodeSP node) {
        foreach(const JobItem &item, nodeStack()) {
            if(item.m_node == node)
                return true;
        }
        return false;
    }
};


#endif /* __KIS_REFRESH_SUBTREE_WALKER_H */

