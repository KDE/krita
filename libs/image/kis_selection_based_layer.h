/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#ifndef KIS_SELECTION_BASED_LAYER_H_
#define KIS_SELECTION_BASED_LAYER_H_

#include <QObject>
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_indirect_painting_support.h"
#include <kritaimage_export.h>
#include "kis_node_filter_interface.h"

class KisFilterConfiguration;

/**
 * @class KisSelectionBasedLayer describes base behaviour for
 * selection base classes like KisAdjustmentLayer and KisGeneratorLayer.
 * These classes should have a persistent selection that controls
 * the area where filter/generators are applied. The area outside
 * this selection is not affected by the layer
 */
class KRITAIMAGE_EXPORT KisSelectionBasedLayer : public KisLayer, public KisIndirectPaintingSupport, public KisNodeFilterInterface
{
    Q_OBJECT

public:
    /**
     * creates a new layer with the given selection.
     * Note that the selection will be _copied_ (with COW, though).
     * @param image the image to set this layer to
     * @param name name of the layer
     * @param selection is a mask used by the layer to know
     * where to apply the filter/generator.
     */
    KisSelectionBasedLayer(KisImageWSP image, const QString &name, KisSelectionSP selection, KisFilterConfigurationSP filterConfig, bool useGeneratorRegistry = false);
    KisSelectionBasedLayer(const KisSelectionBasedLayer& rhs);
    ~KisSelectionBasedLayer() override;


    /**
     * tells whether the @node can be a child of this layer
     * @param node to be connected node
     * @return tells if to be connected is a child of KisMask
     */
    bool allowAsChild(KisNodeSP node) const override;

    void setImage(KisImageWSP image) override;

    KisPaintDeviceSP original() const override;
    KisPaintDeviceSP paintDevice() const override;

    bool needProjection() const override;

    /**
     * resets cached projection of lower layer to a new device
     * @return void
     */
    virtual void resetCache();

    /**
     * for KisLayer::setDirty(const QRegion&)
     */
    using KisLayer::setDirty;

    /**
     * Mark a layer as dirty. We can't use KisLayer's one
     * as our extent() function doesn't fit for this
     */
    void setDirty() override;

public:

    /**
     * Returns the selection of the layer
     *
     * Do not mix it with selection() which returns
     * the currently active selection of the image
     */
    KisSelectionSP internalSelection() const;

    /**
     * sets the selection of this layer to a copy of
     * selection
     * @param selection the selection to set
     * @return void
     */

    void setInternalSelection(KisSelectionSP selection);

    /**
     * When painted in indirect painting mode, the internal selection
     * might not contain actual selection, because a part of it is
     * stored on an indirect painting device. This method returns the
     * merged copy of the real selection. The area in \p rect only is
     * guaranteed to be prepared. The content of the rest of the
     * selection is undefined.
     */
    KisSelectionSP fetchComposedInternalSelection(const QRect &rect) const;

    /**
     * gets this layer's x coordinate, taking selection into account
     * @return x-coordinate value
     */
    qint32 x() const override;

    /**
     * gets this layer's y coordinate, taking selection into account
     * @return y-coordinate value
     */
    qint32 y() const override;

    /**
     * sets this layer's y coordinate, taking selection into account
     * @param x x coordinate
     */
    void setX(qint32 x) override;

    /**
     * sets this layer's y coordinate, taking selection into account
     * @param y y coordinate
     */
    void setY(qint32 y) override;

public:

    /**
     * gets an approximation of where the bounds on actual data
     * are in this layer, taking selection into account
     */
    QRect extent() const override;

    /**
     * returns the exact bounds of where the actual data resides
     * in this layer, taking selection into account
     */
    QRect exactBounds() const override;

    /**
     * copies the image and reformats it to thumbnail size
     * and returns the new thumbnail image.
     * @param w width of the thumbnail to create
     * @param h height of the thumbnail to create
     * @return the thumbnail image created.
     */
    QImage createThumbnail(qint32 w, qint32 h) override;


protected:
    // override from KisLayer
    void copyOriginalToProjection(const KisPaintDeviceSP original,
                                  KisPaintDeviceSP projection,
                                  const QRect& rect) const override;
    // override from KisNode
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;

protected:
    void initSelection();

    QRect cropChangeRectBySelection(const QRect &rect) const;

    /**
     * Sets if the selection should be used in
     * copyOriginalToProjection() method.
     *
     * Default value is 'true'. The descendants should override it to
     * get desired behaviour.
     *
     * Must be called only once in the child's constructor
     */
    void setUseSelectionInProjection(bool value) const;

    KisKeyframeChannel *requestKeyframeChannel(const QString &id) override;

public Q_SLOTS:
    void slotImageSizeChanged();

    /**
     * gets this layer. Overriddes function in
     * KisIndirectPaintingSupport
     * @return this AdjustmentLayer
     */
    KisLayer* layer() {
        return this;
    }

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_SELECTION_BASED_LAYER_H_ */

