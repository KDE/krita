/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.b
 */
#ifndef _KIS_MASK_
#define _KIS_MASK_

#include <QRect>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_node.h"
#include "kis_indirect_painting_support.h"

#include <kritaimage_export.h>

/**
 KisMask is the base class for all single channel
 mask-like paint devices in Krita. Masks can be rendered in different
 ways at different moments during the rendering stack. Masks are
 "owned" by layers (of any type), and cannot occur by themselves on
 themselves.

 The properties that masks implement are made available through the
 iterators created on their parent layer, or through iterators that
 can be created on the paint device that holds the mask data: masks
 are just paint devices, too.

 Masks should show up in the layerbox as sub-layers for the layer they
 are associated with and be ccp'able and draggable to other layers.

 Examples of masks are:

 - filter masks: like the alpha filter mask that is the most common
                 type of mask and is simply known as "mask" in the
                 gui. Other filter masks use any of krita's filters to
                 filter the pixels of their parent. (In this they
                 differ from adjustment layers, which filter all
                 layers under them in their group stack).

 - selections: the selection mask is rendered after composition and
   zooming and determines the selectedness of the pixels of the parent
   layer.

 - painterly overlays: painterly overlays indicate a particular
   property of the pixel in the parent paint device they are associated
   with, like wetness, height or gravity.

   XXX: For now, all masks are 8 bit. Make the channel depth settable.

 */
class KRITAIMAGE_EXPORT KisMask : public KisNode, public KisIndirectPaintingSupport
{

    Q_OBJECT

public:

    /**
     * Create a new KisMask.
     */
    KisMask(const QString & name);

    /**
     * Copy the mask
     */
    KisMask(const KisMask& rhs);

    ~KisMask() override;

    void setImage(KisImageWSP image) override;

    bool allowAsChild(KisNodeSP node) const override;

    /**
     * @brief initSelection initializes the selection for the mask from
     *   the given selection's projection.
     * @param copyFrom the selection we base the mask on
     * @param parentLayer the parent of this mask; it determines the default bounds of the mask.
     */
    void initSelection(KisSelectionSP copyFrom, KisLayerSP parentLayer);

    /**
     * @brief initSelection initializes the selection for the mask from
     *   the given paint device.
     * @param copyFromDevice the paint device we base the mask on
     * @param parentLayer the parent of this mask; it determines the default bounds of the mask.
     */
    void initSelection(KisPaintDeviceSP copyFromDevice, KisLayerSP parentLayer);

    /**
     * @brief initSelection initializes an empty selection
     * @param parentLayer the parent of this mask; it determines the default bounds of the mask.
     */
    void initSelection(KisLayerSP parentLayer);

    const KoColorSpace * colorSpace() const override;
    const KoCompositeOp * compositeOp() const override;

    /**
     * Return the selection associated with this mask. A selection can
     * contain both a paint device and shapes.
     */
    KisSelectionSP selection() const;

    /**
     * @return the selection: if you paint on mask, you paint on the selections
     */
    KisPaintDeviceSP paintDevice() const override;

    /**
     * @return the same as paintDevice()
     */
    KisPaintDeviceSP original() const override;

    /**
     * @return the same as paintDevice()
     */
    KisPaintDeviceSP projection() const override;

    KisAbstractProjectionPlaneSP projectionPlane() const override;

    /**
     * Change the selection to the specified selection object. The
     * selection is deep copied.
     */
    void setSelection(KisSelectionSP selection);

    /**
     * Selected the specified rect with the specified amount of selectedness.
     */
    void select(const QRect & rc, quint8 selectedness = MAX_SELECTED);

    /**
     * The extent and bounds of the mask are those of the selection inside
     */
    QRect extent() const override;
    QRect exactBounds() const override;

    /**
     * overridden from KisBaseNode
     */
    qint32 x() const override;

    /**
     * overridden from KisBaseNode
     */
    void setX(qint32 x) override;

    /**
     * overridden from KisBaseNode
     */
    qint32 y() const override;

    /**
     * overridden from KisBaseNode
     */
    void setY(qint32 y) override;

    /**
     * Usually masks themselves do not have any paint device and
     * all their final effect on the layer stack is computed using
     * the changeRect() of the dirty rect of the parent layer. Their
     * extent() and exectBounds() methods work the same way: by taking
     * the extent of the parent layer and computing the rect basing
     * on it. But some of the masks like Colorize Mask may have their
     * own "projection", which is painted independently from the changed
     * area of the parent layer. This additional "non-dependent" extent
     * is added to the extent of the parent layer.
     */
    virtual QRect nonDependentExtent() const;

    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;
    QRect changeRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;
    QImage createThumbnail(qint32 w, qint32 h) override;

    void testingInitSelection(const QRect &rect, KisLayerSP parentLayer);

protected:
    /**
     * Apply the effect the projection using the mask as a selection.
     * Made public in KisEffectMask
     */
    void apply(KisPaintDeviceSP projection, const QRect & applyRect, const QRect & needRect, PositionToFilthy maskPos) const;

    virtual void mergeInMaskInternal(KisPaintDeviceSP projection,
                                     KisSelectionSP effectiveSelection,
                                     const QRect &applyRect, const QRect &preparedNeedRect,
                                     PositionToFilthy maskPos) const;

    /**
     * A special callback for calling selection->updateProjection() during
     * the projection calculation process. Some masks (e.g. selection masks)
     * don't need it, because they do it separately.
     */
    virtual void flattenSelectionProjection(KisSelectionSP selection, const QRect &dirtyRect) const;

    virtual QRect decorateRect(KisPaintDeviceSP &src,
                               KisPaintDeviceSP &dst,
                               const QRect & rc,
                               PositionToFilthy maskPos) const;

    virtual bool paintsOutsideSelection() const;

    KisKeyframeChannel *requestKeyframeChannel(const QString &id) override;

    void baseNodeChangedCallback() override;

private:
    friend class KisMaskProjectionPlane;

private:

    struct Private;

    Private * const m_d;

};


#endif
