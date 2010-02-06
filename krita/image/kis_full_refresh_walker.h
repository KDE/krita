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

#ifndef __KIS_FULL_REFRESH_WALKER_H
#define __KIS_FULL_REFRESH_WALKER_H

#include "kis_node.h"
#include "kis_types.h"
#include "kis_base_rects_walker.h"

class KRITAIMAGE_EXPORT KisFullRefreshWalker : public KisBaseRectsWalker
{

public:
    KisFullRefreshWalker(QRect cropRect)
        : KisBaseRectsWalker(cropRect)
    {
    }

    virtual ~KisFullRefreshWalker()
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
    void startTrip(KisNodeSP startWith) {
        /**
         * We don't even bother about change rect, because
         * it is gracefully set up to requestedRect by the base class.
         * Anyway first invocation of startTrip will usually
         * have startWith set to root layer
         */

        // Just for consistency reasons
        if(isRootNode(startWith))
            registerNeedRect(startWith, N_TOPMOST);

        KisNodeSP currentNode = startWith->lastChild();
        if(!currentNode) return;

        registerNeedRect(currentNode, N_TOPMOST);

        while ((currentNode = currentNode->prevSibling())) {
            registerNeedRect(currentNode, N_NORMAL);
        }

        currentNode = startWith->lastChild();
        do {
            if(canHaveChildLayers(currentNode))
                startTrip(currentNode);
        } while ((currentNode = currentNode->prevSibling()));
    }
};


#endif /* __KIS_FULL_REFRESH_WALKER_H */

