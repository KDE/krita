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

#ifndef __KIS_GRAPH_WALKER_H
#define __KIS_GRAPH_WALKER_H

#include "kis_node.h"
#include "kis_types.h"

class KRITAIMAGE_EXPORT KisGraphWalker
{
public:
    enum NodePosition {
        N_NORMAL,
        N_LOWER,
        N_TOPMOST,
        N_BOTTOMMOST
    };

public:
    KisGraphWalker();
    virtual ~KisGraphWalker();

    /**
     * Begins visiting nodes starting with @startWith.
     * First it climbs to the top of the graph, collecting
     * changeRects (it calls @registerChangeRect for every node).
     * Then it goes down to the bottom collecting needRects
     * for every branch.
     */
    void startTrip(KisNodeSP startWith);

private:
    /**
     * Visits a node @node and goes on crowling
     * towards the top of the graph, caling visitHigherNode() or
     * startTrip() one more time. After the top is reached
     * returns back to the @node.
     */
    void visitHigherNode(KisNodeSP node);

    /**
     * Visits a node @node and goes on crowling
     * towards the bottom of the graph, caling visitLowerNode() or
     * startTrip() one more time.
     */
    void visitLowerNode(KisNodeSP node);

protected:
    /**
     * Repeats iterations of changeRect-loop until reaches @targetNode.
     * It's O(n) but is supposed to be used very rarely. It will be called
     * for nodes lying under a node with empty needRect, but above
     * filthy node.
     */
    QRect getChangeRectForNode(const KisNodeSP &targetNode,
                               const KisNodeSP &startNode,
                               const QRect &startRect);

    /**
     * Called for every node we meet on a forward way of the trip.
     */
    virtual void registerChangeRect(KisNodeSP node) = 0;

    /**
     * Called for every node we meet on a backward way of the trip.
     */
    virtual void registerNeedRect(KisNodeSP node,
                                  NodePosition position) = 0;
};


#endif /* __KIS_GRAPH_WALKER_H */

