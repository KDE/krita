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
 * @file
 * This file is part of the Krita koffice application. It handles
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
#include <krita_export.h>
#include "kis_selection_based_layer.h"


class KisFilterConfiguration;

/**
 * @class KisAdjustmentLayer Contains a KisFilter and a KisSelection.
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
    KisAdjustmentLayer(KisImageWSP image, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection);
    KisAdjustmentLayer(const KisAdjustmentLayer& rhs);
    virtual ~KisAdjustmentLayer();

    bool accept(KisNodeVisitor &);

    /**
     * clones this AdjustmentLayer into a KisNodeSP type.
     * @return the KisNodeSP returned
     */
    KisNodeSP clone() const {
        return KisNodeSP(new KisAdjustmentLayer(*this));
    }

    /**
     * gets the adjustmentLayer's tool filter
     * @return QIcon returns the KIcon tool filter
     */
    QIcon icon() const;

    /**
     * gets the AdjustmentLayer properties describing whether
     * or not the node is locked, visible, and the filter
     * name is it is a filter. Overrides sectionModelProperties
     * in KisLayer, and KisLayer overrides
     * sectionModelProperties in KisBaseNode.
     * @return KoDocumentSectionModel::PropertyList returns a list
     * of the properties
     */
    KoDocumentSectionModel::PropertyList sectionModelProperties() const;

public:

    /**
     * gets a pointer to the AdjustmentLayer's filter configuration.
     * @return a pointer to the AdjustmentLayer's filter configuration
     */
    KisFilterConfiguration *filter() const;

    /**
     * sets the AdjustmentLayer's filter configuration
     * @param filterConfig a pointer to the filter configuration to set
     * @return void
     */
    void setFilter(KisFilterConfiguration *filterConfig);

public slots:
    /**
     * gets this AdjustmentLayer. Overriddes function in
     * KisIndirectPaintingSupport
     * @return this AdjustmentLayer
     */
    KisLayer* layer() {
        return this;
    }

private:
    class Private;
    Private * const m_d;
};

#endif // KIS_ADJUSTMENT_LAYER_H_

