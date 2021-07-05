/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    /**
     * A leaf can be renderable even when it is invisible, e.g. when it has
     * clones (it should update data for clones)
     */
    bool shouldBeRendered() const;

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
