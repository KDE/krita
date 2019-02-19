/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

/**
 * @file kis_adjustment_layer.h
 * This file is part of the Krita calligra application. It handles
 * a contains a KisFilter OR a KisLayer, and this class is created
 * to influence the rendering of layers below this one. Can also
 * function as a fixating layer.
 *
 * @author Boudewijn Rempt
 * @author comments by hscott
 * @since 1.5
 */
#ifndef KIS_ADJUSTMENT_LAYER_H_
#define KIS_ADJUSTMENT_LAYER_H_

#include <QObject>
#include <kritaimage_export.h>
#include "kis_selection_based_layer.h"


class KisFilterConfiguration;

/**
 * @class KisAdjustmentLayer
 * @brief Contains a KisFilter and a KisSelection.
 * 
 * If the selection is present, it is a mask used by the adjustment layer
 * to know where to apply the filter, thus the  combination is used
 * to influence the rendering of the layers under this layer
 * in the layerstack. AdjustmentLayers also function as a kind
 * of "fixating layers".
 */
class KRITAIMAGE_EXPORT KisAdjustmentLayer : public KisSelectionBasedLayer
{
    Q_OBJECT

public:
    /**
     * creates a new adjustment layer with the given
     * configuration and selection. Note that the selection
     * will be _copied_ (with COW, though).
     * @param image the image to set this AdjustmentLayer to
     * @param name name of the adjustment layer
     * @param kfc the configuration for the adjustment layer filter
     * @param selection is a mask used by the adjustment layer to
     * know where to apply the filter.
     */
    KisAdjustmentLayer(KisImageWSP image, const QString &name, KisFilterConfigurationSP  kfc, KisSelectionSP selection);
    KisAdjustmentLayer(const KisAdjustmentLayer& rhs);
    ~KisAdjustmentLayer() override;

    bool accept(KisNodeVisitor &) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    /**
     * clones this AdjustmentLayer into a KisNodeSP type.
     * @return the KisNodeSP returned
     */
    KisNodeSP clone() const override {
        return KisNodeSP(new KisAdjustmentLayer(*this));
    }

    /**
     * gets the adjustmentLayer's tool filter
     * @return QIcon returns the QIcon tool filter
     */
    QIcon icon() const override;

    /**
     * gets the AdjustmentLayer properties describing whether
     * or not the node is locked, visible, and the filter
     * name is it is a filter. Overrides sectionModelProperties
     * in KisLayer, and KisLayer overrides
     * sectionModelProperties in KisBaseNode.
     * @return KisBaseNode::PropertyList returns a list
     * of the properties
     */
    KisBaseNode::PropertyList sectionModelProperties() const override;

public:

    /**
     * \see KisNodeFilterInterface::setFilter()
     */
    void setFilter(KisFilterConfigurationSP filterConfig) override;

    void setChannelFlags(const QBitArray & channelFlags) override;

protected:
    // override from KisLayer
    QRect incomingChangeRect(const QRect &rect) const override;
    // override from KisNode
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;

public Q_SLOTS:
    /**
     * gets this AdjustmentLayer. Overrides function in
     * KisIndirectPaintingSupport
     * @return this AdjustmentLayer
     */
    KisLayer* layer() {
        return this;
    }
};

#endif // KIS_ADJUSTMENT_LAYER_H_

