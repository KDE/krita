/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FULL_REFRESH_WALKER_H
#define __KIS_FULL_REFRESH_WALKER_H

#include "kis_merge_walker.h"
#include "kis_refresh_subtree_walker.h"


class KisFullRefreshWalker : public KisRefreshSubtreeWalker, public KisMergeWalker
{
public:
    KisFullRefreshWalker(QRect cropRect)
        : KisMergeWalker(NO_FILTHY)
    {
        setCropRect(cropRect);
    }

    UpdateType type() const override {
        return FULL_REFRESH;
    }

    void startTrip(KisProjectionLeafSP startWith) override {
        if(m_firstRun) {
            m_firstRun = false;

            m_currentUpdateType = UPDATE;
            KisMergeWalker::startTrip(startWith);

            m_currentUpdateType = FULL_REFRESH;
            KisRefreshSubtreeWalker::startTrip(startWith);

            m_firstRun = true;
        }
        else {
            if(m_currentUpdateType == FULL_REFRESH) {
                KisRefreshSubtreeWalker::startTrip(startWith);
            }
            else {
                KisMergeWalker::startTrip(startWith);
            }
        }
    }

    void registerChangeRect(KisProjectionLeafSP leaf, NodePosition position) override {
        if(m_currentUpdateType == FULL_REFRESH) {
            KisRefreshSubtreeWalker::registerChangeRect(leaf, position);
        }
        else {
            /**
             * Merge walker thinks that we changed the original of the
             * dirty node (dirtyNode == startNode()), but that is not
             * true in case of full refresh walker, because all the
             * children of the dirty node are dirty as well, that is
             * why we shouldn't rely on usual registerChangeRect()
             * mechanism for this node. That is why we just unite the
             * changeRects of all its children here.
             */

            if(isStartLeaf(leaf)&& !leaf->isRoot()) {
                KisRefreshSubtreeWalker::calculateChangeRect(leaf, requestedRect());
            }
            else {
                KisMergeWalker::registerChangeRect(leaf, position);
            }
        }
    }
    void registerNeedRect(KisProjectionLeafSP leaf, NodePosition position) override {
        if(m_currentUpdateType == FULL_REFRESH) {
            KisRefreshSubtreeWalker::registerNeedRect(leaf, position);
        }
        else {
            KisMergeWalker::registerNeedRect(leaf, position);
        }
    }
    void adjustMasksChangeRect(KisProjectionLeafSP firstMask) override {
        if(m_currentUpdateType == FULL_REFRESH) {
            KisRefreshSubtreeWalker::adjustMasksChangeRect(firstMask);
        }
        else {
            KisMergeWalker::adjustMasksChangeRect(firstMask);
        }
    }

private:
    UpdateType m_currentUpdateType { UPDATE };
    bool m_firstRun { true };
};

#endif /* __KIS_FULL_REFRESH_WALKER_H */
