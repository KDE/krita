/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_REFRESH_SUBTREE_WALKER_H
#define __KIS_REFRESH_SUBTREE_WALKER_H

#include "kis_types.h"
#include "kis_base_rects_walker.h"


class KRITAIMAGE_EXPORT KisRefreshSubtreeWalker : public virtual KisBaseRectsWalker
{

public:
    KisRefreshSubtreeWalker(QRect cropRect, bool skipNonRenderableNodes = false)
        : m_skipNonRenderableNodes(skipNonRenderableNodes)
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

        /**
         * If the node is not renderable and we don't care about hidden groups,
         * e.g. when generating animation frames, then just skip the entire group.
         */
        if (m_skipNonRenderableNodes && !startWith->shouldBeRendered()) return;


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

private:
    bool m_skipNonRenderableNodes = false;
};


#endif /* __KIS_REFRESH_SUBTREE_WALKER_H */

