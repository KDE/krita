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

#ifndef __KIS_MERGE_WALKER_H
#define __KIS_MERGE_WALKER_H

#include "kis_types.h"
#include "kis_base_rects_walker.h"

class KisMergeWalker;
typedef KisSharedPtr<KisMergeWalker> KisMergeWalkerSP;

class KRITAIMAGE_EXPORT KisMergeWalker : public virtual KisBaseRectsWalker
{

public:
    /**
     * NO_FILTHY flag notifies the walker that there should be no (!)
     * filthy node in the update. It means that the projection() of
     * the node is already guaranteed to be ready, we just need to
     * update all the higher-level nodes. Used by KisTransformMask
     * regeneration code.
     */
    enum Flags {
        DEFAULT = 0,
        NO_FILTHY
    };

    KisMergeWalker(QRect cropRect, Flags flags = DEFAULT);

    ~KisMergeWalker() override;

    UpdateType type() const override;

protected:
    KisMergeWalker() : m_flags(DEFAULT) {}
    KisMergeWalker(Flags flags) : m_flags(flags) {}

    /**
     * Begins visiting nodes starting with @p startWith.
     * First it climbs to the top of the graph, collecting
     * changeRects (it calls @ref registerChangeRect for every node).
     * Then it goes down to the bottom collecting needRects
     * for every branch.
     */
    void startTrip(KisProjectionLeafSP startWith) override;

    using KisBaseRectsWalker::startTrip;

    void startTripWithMask(KisProjectionLeafSP filthyMask, KisMergeWalker::Flags flags);

private:
    void startTripImpl(KisProjectionLeafSP startLeaf, Flags flags);

private:
    /**
     * Visits a node @leaf and goes on crowling
     * towards the top of the graph, caling visitHigherNode() or
     * startTrip() one more time. After the top is reached
     * returns back to the @leaf.
     */
    void visitHigherNode(KisProjectionLeafSP leaf, NodePosition positionToFilthy);

    /**
     * Visits a node @leaf and goes on crowling
     * towards the bottom of the graph, caling visitLowerNode() or
     * startTrip() one more time.
     */
    void visitLowerNode(KisProjectionLeafSP leaf);

private:
    const Flags m_flags;
};


#endif /* __KIS_MERGE_WALKER_H */

