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
    enum Flag {
        None = 0x0,
        SkipNonRenderableNodes = 0x1,
        NoFilthyMode = 0x2
    };

    Q_DECLARE_FLAGS(Flags, Flag);

public:
    KisRefreshSubtreeWalker(QRect cropRect, Flags flags = None)
        : m_flags(flags)
    {
        setCropRect(cropRect);
    }

    UpdateType type() const override {
        return UNSUPPORTED;
    }

    ~KisRefreshSubtreeWalker() override
    {
    }

    Flags flags() const {
        return m_flags;
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

            if(currentLeaf->isLayer() && currentLeaf->shouldBeRendered()) {
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
        if (m_flags & SkipNonRenderableNodes && !startWith->shouldBeRendered()) return;


        KisProjectionLeafSP currentLeaf = startWith->lastChild();
        while(currentLeaf) {
            NodePosition pos = (m_flags & NoFilthyMode ? N_ABOVE_FILTHY : N_FILTHY) |
                calculateNodePosition(currentLeaf);
            registerNeedRect(currentLeaf, pos);

            // see a comment above
            registerCloneNotification(currentLeaf->node(), pos);
            currentLeaf = currentLeaf->prevSibling();
        }

        /**
         * In no-filthy mode we just recompose the root layer
         * without entering any subgroups
         */
        if (m_flags & NoFilthyMode) return;

        currentLeaf = startWith->lastChild();
        while(currentLeaf) {
            if(currentLeaf->canHaveChildLayers()) {
                startTrip(currentLeaf);
            }
            currentLeaf = currentLeaf->prevSibling();
        }
    }

private:
    Flags m_flags = None;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisRefreshSubtreeWalker::Flags);

#endif /* __KIS_REFRESH_SUBTREE_WALKER_H */

