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

#include "kis_types.h"
#include "kis_base_rects_walker.h"


class KRITAIMAGE_EXPORT KisRefreshSubtreeWalker : public virtual KisBaseRectsWalker
{

public:
    KisRefreshSubtreeWalker(QRect cropRect)
    {
        setCropRect(cropRect);
    }

    UpdateType type() const override {
        return UNSUPPORTED;
    }

    ~KisRefreshSubtreeWalker() override
    {
    }

protected:
    KisRefreshSubtreeWalker() {}


    QRect calculateChangeRect(KisProjectionLeafSP startWith,
                              const QRect &requestedRect) {

        if(!startWith->isLayer())
            return requestedRect;

        QRect childrenRect;
        QRect tempRect = requestedRect;
        bool changeRectVaries = false;

        KisProjectionLeafSP currentLeaf = startWith->firstChild();
        KisProjectionLeafSP prevLeaf;
        KisProjectionLeafSP nextLeaf;

        while(currentLeaf) {
            nextLeaf = currentLeaf->nextSibling();

            if(currentLeaf->isLayer()) {
                tempRect |= calculateChangeRect(currentLeaf, requestedRect);

                if(!changeRectVaries)
                    changeRectVaries = tempRect != requestedRect;

                childrenRect = tempRect;
                prevLeaf = currentLeaf;
            }

            currentLeaf = nextLeaf;
        }

        tempRect |= startWith->projectionPlane()->changeRect(requestedRect | childrenRect);

        if(!changeRectVaries)
            changeRectVaries = tempRect != requestedRect;

        setExplicitChangeRect(tempRect, changeRectVaries);

        return tempRect;
    }

    void startTrip(KisProjectionLeafSP startWith) override {
        setExplicitChangeRect(requestedRect(), false);

        if (isStartLeaf(startWith)) {
            KisProjectionLeafSP extraUpdateLeaf = startWith;

            if (startWith->isMask()) {
                /**
                 * When the mask is the root of the update, update
                 * its parent projection using N_EXTRA method.
                 *
                 * This special update is necessary because the following
                 * wolker will work in N_ABOVE_FILTHY mode only
                 */

                extraUpdateLeaf = startWith->parent();
            }

            /**
             * Sometimes it may happen that the mask is placed outside layers hierarchy
             * (e.g. inactive selection mask), then the projection leafs will not point
             * to anywhere
             */
            if (extraUpdateLeaf) {
                NodePosition pos = N_EXTRA | calculateNodePosition(extraUpdateLeaf);
                registerNeedRect(extraUpdateLeaf, pos);

                /**
                 * In normal walkers we register notifications
                 * in the change-rect pass to avoid regeneration
                 * of the nodes that are below filthy. In the subtree
                 * walker there is no change-rect pass and all the
                 * nodes are considered as filthy, so we should do
                 * that explicitly.
                 */
                registerCloneNotification(extraUpdateLeaf->node(), pos);
            }
        }

        KisProjectionLeafSP currentLeaf = startWith->lastChild();
        while(currentLeaf) {
            NodePosition pos = N_FILTHY | calculateNodePosition(currentLeaf);
            registerNeedRect(currentLeaf, pos);

            // see a comment above
            registerCloneNotification(currentLeaf->node(), pos);
            currentLeaf = currentLeaf->prevSibling();
        }

        currentLeaf = startWith->lastChild();
        while(currentLeaf) {
            if(currentLeaf->canHaveChildLayers()) {
                startTrip(currentLeaf);
            }
            currentLeaf = currentLeaf->prevSibling();
        }
    }
};


#endif /* __KIS_REFRESH_SUBTREE_WALKER_H */

