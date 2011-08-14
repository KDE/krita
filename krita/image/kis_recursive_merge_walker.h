/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_RECURSIVE_MERGE_WALKER_H
#define __KIS_RECURSIVE_MERGE_WALKER_H

#include "kis_merge_walker.h"
#include "kis_full_refresh_walker.h"


class KisRecursiveMergeWalker : public KisFullRefreshWalker, public KisMergeWalker
{
public:
    KisRecursiveMergeWalker(QRect cropRect)
        : m_firstRun(true)
    {
        setCropRect(cropRect);
    }

    UpdateType type() const {
        return FULL_REFRESH;
    }

    void startTrip(KisNodeSP startWith) {
        if(m_firstRun) {
            m_firstRun = false;

            m_currentUpdateType = UPDATE;
            KisMergeWalker::startTrip(startWith);

            m_currentUpdateType = FULL_REFRESH;
            KisFullRefreshWalker::startTrip(startWith);

            m_firstRun = true;
        }
        else {
            if(m_currentUpdateType == FULL_REFRESH) {
                KisFullRefreshWalker::startTrip(startWith);
            }
            else {
                KisMergeWalker::startTrip(startWith);
            }
        }
    }

    void registerChangeRect(KisNodeSP node, NodePosition position) {
        if(m_currentUpdateType == FULL_REFRESH) {
            KisFullRefreshWalker::registerChangeRect(node, position);
        }
        else {
            KisMergeWalker::registerChangeRect(node, position);
        }
    }
    void registerNeedRect(KisNodeSP node, NodePosition position) {
        if(m_currentUpdateType == FULL_REFRESH) {
            KisFullRefreshWalker::registerNeedRect(node, position);
        }
        else {
            KisMergeWalker::registerNeedRect(node, position);
        }
    }
    void adjustMasksChangeRect(KisNodeSP firstMask) {
        if(m_currentUpdateType == FULL_REFRESH) {
            KisFullRefreshWalker::adjustMasksChangeRect(firstMask);
        }
        else {
            KisMergeWalker::adjustMasksChangeRect(firstMask);
        }
    }

private:
    UpdateType m_currentUpdateType;
    bool m_firstRun;
};

#endif /* __KIS_RECURSIVE_MERGE_WALKER_H */
