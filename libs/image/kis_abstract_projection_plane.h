/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ABSTRACT_PROJECTION_PLANE_H
#define __KIS_ABSTRACT_PROJECTION_PLANE_H

#include "kis_types.h"
#include "kis_layer.h"

class QRect;
class KisPainter;


/**
 * An interface of the node to the compositioning
 * system. Compositioning system KisAsyncMerger knows nothing about
 * the internals of the layer, it just knows that the layer can:
 *
 * 1) recalculate() its internal representation if it is filthy
 *
 * 2) apply() itself onto a global projection using, writing all its data
 *            to the projection.
 *
 * 3) report parameters of these operations using needRect(),
 *    changeRect() and accessRect() methods, which mean the same as
 *    the corresponding methods of KisNode, but include more
 *    transformations for the layer, e.g. Layer Styles and
 *    Pass-through mode.
 */
class KRITAIMAGE_EXPORT  KisAbstractProjectionPlane
{
public:
    KisAbstractProjectionPlane();
    virtual ~KisAbstractProjectionPlane();

    /**
     * Is called by the async merger when the node is filthy and
     * should recalculate its internal representation. For usual
     * layers it means just calling updateProjection().
     */
    virtual QRect recalculate(const QRect& rect, KisNodeSP filthyNode) = 0;

    /**
     * Writes the data of the projection plane onto a global
     * projection using \p painter object.
     */
    virtual void apply(KisPainter *painter, const QRect &rect) = 0;

    /**
     * Works like KisNode::needRect(), but includes more
     * transformations of the layer
     */
    virtual QRect needRect(const QRect &rect, KisLayer::PositionToFilthy pos = KisLayer::N_FILTHY) const = 0;

    /**
     * Works like KisNode::changeRect(), but includes more
     * transformations of the layer
     */
    virtual QRect changeRect(const QRect &rect, KisLayer::PositionToFilthy pos = KisLayer::N_FILTHY) const = 0;

    /**
     * Works like KisNode::needRect(), but includes more
     * transformations of the layer
     */
    virtual QRect accessRect(const QRect &rect, KisLayer::PositionToFilthy pos = KisLayer::N_FILTHY) const = 0;

    /**
     * Works like KisLayer::needRectForOriginal(), but includes needed
     * rects of layer styles
     */
    virtual QRect needRectForOriginal(const QRect &rect) const = 0;

    /**
     * Return a tight rectangle, where the contents of the plane
     * is placed from user's point of view. It includes everything
     * belonging to the plane (e.g. layer styles).
     */
    virtual QRect tightUserVisibleBounds() const = 0;

    /**
     * Returns a list of devices which should synchronize the lod cache on update
     */
    virtual KisPaintDeviceList getLodCapableDevices() const = 0;
};

/**
 * A simple implementation of KisAbstractProjectionPlane interface
 * that does nothing.
 */
class KisDumbProjectionPlane : public KisAbstractProjectionPlane
{
public:
    QRect recalculate(const QRect& rect, KisNodeSP filthyNode) override;
    void apply(KisPainter *painter, const QRect &rect) override;

    QRect needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const override;
    QRect needRectForOriginal(const QRect &rect) const override;
    QRect tightUserVisibleBounds() const override;

    KisPaintDeviceList getLodCapableDevices() const override;
};

#endif /* __KIS_ABSTRACT_PROJECTION_PLANE_H */
