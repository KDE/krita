/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PROJECTION_LEAF_H
#define __KIS_PROJECTION_LEAF_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kritaimage_export.h"

class KisNodeVisitor;


class KRITAIMAGE_EXPORT KisProjectionLeaf
{
public:
    KisProjectionLeaf(KisNode *node);
    virtual ~KisProjectionLeaf();

    KisProjectionLeafSP parent() const;

    KisProjectionLeafSP firstChild() const;
    KisProjectionLeafSP lastChild() const;

    KisProjectionLeafSP prevSibling() const;
    KisProjectionLeafSP nextSibling() const;

    KisNodeSP node() const;
    KisAbstractProjectionPlaneSP projectionPlane() const;
    bool accept(KisNodeVisitor &visitor);

    KisPaintDeviceSP original();
    KisPaintDeviceSP projection();

    bool isRoot() const;
    bool isLayer() const;
    bool isMask() const;
    bool canHaveChildLayers() const;
    bool dependsOnLowerNodes() const;
    bool visible() const;
    quint8 opacity() const;
    QBitArray channelFlags() const;
    bool isStillInGraph() const;
    bool hasClones() const;

    bool isDroppedNode() const;

    enum NodeDropReason {
        NodeAvailable,
        DropPassThroughMask,
        DropPassThroughClone
    };
    NodeDropReason dropReason() const;

    bool isOverlayProjectionLeaf() const;

    /**
     * Temporarily exclude the projection leaf from rendering by making
     * it invisible (KisProjectionLeaf::visible() == false).
     *
     * This method is used by the tools that want to hide the
     * original layer's content temporarily.
     *
     * NOTE: the method is not thread-safe! The caller must guarantee
     * exclusive access to the projection leaf himself.
     */
    void setTemporaryHiddenFromRendering(bool value);

    /**
     * \see setTemporaryHiddenFromRendering
     */
    bool isTemporaryHiddenFromRendering() const;

    /**
     * Regenerate projection of the current group layer iff it is
     * pass-through mode.
     *
     * WARNING: must be called either under the image lock held
     *          or in the context of an exclusive stroke job.
     */
    void explicitlyRegeneratePassThroughProjection();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_PROJECTION_LEAF_H */
