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
#include "kis_types.h"
#include "kis_layer.h"
#include <krita_export.h>
#include "kis_node_filter_interface.h"

class KisFilterConfiguration;
class KisNodeVisitor;

 /** XXX_NODE: implement prepareForRemoval with:
// Adjustment layers should mark the layers underneath 
// them, whose rendering they have cached, dirty on 
// removal. Otherwise, the group won't be re-rendered.

   KisAdjustmentLayer * al = 
      dynamic_cast<KisAdjustmentLayer*>(layer.data());
   if (al) 
   {
      QRect r = al->extent();
      // Lock the image, because we are going to dirty 
      // a lot of layers 
      lock(); 
      KisLayerSP l = layer->nextSibling();
      while (l)
      {
         KisAdjustmentLayer * al2 = 
            dynamic_cast<KisAdjustmentLayer*>(l.data());
         if (al2 != 0) break;
         l = l->nextSibling();
   }
   unlock();
	}

 * XXX_NODE: also implement masks modifying the adj. layer's selection.
 */

/**
 * @class KisAdjustmentLayer Contains a KisFilter and optionally a 
 * KisSelection. If the selection is present, it is a mask used by
 * the adjustment layer to know where to apply the filter, thus the 
 * combination is used to influence the rendering of the layers 
 * under this layer in the layerstack. AdjustmentLayers also 
 * function as a kind of "fixating layers".
 */
class KRITAIMAGE_EXPORT KisAdjustmentLayer : public KisLayer, public KisIndirectPaintingSupport, public KisNodeFilterInterface
{
    Q_OBJECT

public:
    /**
     * creates a new adjustment layer with the given 
     * configuration and selection. Note that the selection 
     * will be _copied_.
     * @param img the image to set this AdjustmentLayer to
     * @param name name of the adjustment layer 
     * @param kfc the configuration for the adjustment layer filter
     * @param selection is a mask used by the adjustment layer to 
     * know where to apply the filter.
     */
    KisAdjustmentLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection);
    KisAdjustmentLayer(const KisAdjustmentLayer& rhs);
    virtual ~KisAdjustmentLayer();

    /**
     * clones this AdjustmentLayer into a KisNodeSP type.
     * @return the KisNodeSP returned
     */		
    KisNodeSP clone() const {
        return KisNodeSP(new KisAdjustmentLayer(*this));
    }

    /**
     * returns true if this AdjustmentLayer inherits KisMask. This
     * is important because only certain layers can have "effect masks".
     * And only LayerGroup can have other layers as children.
     * @param node a clone of this AdjustmentLayer.
     * @return tells if this AdjustmentLayer is a child of KisMask
     */
    bool allowAsChild(KisNodeSP) const;

    /**
     * sets the QRect passed to this function as an object not in use
     * using the Qt macro Q_UNUSED
     * @param r Q_UNUSED is set to r  
     * @return void
     */
    void updateProjection(const QRect& r);

    /**
     * gets the final result of the layer and all masks. 
     * @return m_d->cachedPaintDevice KisPaintDeviceSP the 
     * final result of the layer and all masks
     */
    KisPaintDeviceSP projection() const;

    /**
     * gets the paint device for this AdjustmentLayer. 
     * @return paint device that the user can paint on. For 
     * paint layers, this is the basic, wet painting device, 
     * for adjustment layers it's the selection.
     */
    KisPaintDeviceSP paintDevice() const;

    /**
     * gets the adjustmentLayer's tool filter
     * @return QIcon returns the KIcon tool filter
     */
    QIcon icon() const;

    /**
     * gets the AdjustmentLayer properties describing whether 
     * or not the node is locked, visible, and the filter 
     * name is it is a filter. overrides sectionModelProperties 
     * in KisLayer, and KisLayer overrides 
     * sectionModelProperties in KisBaseNode. 
     * @return KoDocumentSectionModel::PropertyList returns a list
     * of 
     */
    KoDocumentSectionModel::PropertyList sectionModelProperties() const;

    using KisLayer::setDirty;
    /**
     * sets the AdjustementLayer's to dirty with the selections 
     * selectedExtractRect() if the AdjustmentLayer is a selection, 
     * otherwise, it is set to dirty without passing it a parameter. 
     * @return 
     */
    virtual void setDirty();

public:

    /**
     * gets a pointer to the AdjustmentLayer's filter configuration. 
     * @return a pointer to the AdjustmentLayer's 
     * FilterConfiguration 
     */
    KisFilterConfiguration * filter() const;

    /**
     * sets the AdjustmentLayer's FilterConfiguration 
     * @param filterConfig a pointer to the FilterConfiguration to set 
     * @return void
     */
    void setFilter(KisFilterConfiguration * filterConfig);

    /**
     * gets this AdjustmentLayer's selection  
     * @return the AdjustmentLayer's selection
     */
    KisSelectionSP selection() const;

    /**
     * sets the selection of this AdjustmentLayer to a copy of
     * selection 
     * @param selection the selection to set 
     * @return void
     */
    void setSelection(KisSelectionSP selection);

    /**
     * gets this AdjustmentLayer's x coordinate if the layer is
     * a selection. Returns 0 if the AdjustmentLayer is not of 
     * selection type. Overridden from KisBaseNode. 
     * @return x-koordinate value or zero
     */
    qint32 x() const;

    /**
     * sets the X coordinate of the AdjustmentLayer if the layer
     * is of selectionType. Otherwise the function does nothing. 
     * Overridden from KisBaseNode
     * @param x the x-coordinate value to set the selection's 
     * x-coordinate to.
     * @return void
     */
    void setX(qint32 x);

    /**
     * gets the AdjustmentLayer's y-coordinate value if the layer
     * is of selection type. Otherwise it returns zero. Overridden 
     * from KisBaseNode.
     * @return the y-coordinate value of the selection
     */
    qint32 y() const;

    /**
     * sets the AdjustmentLayers y-coordinate value if the layer
     * is of selection type. Otherwise the function does nothing.
     * Overridden from KisBaseNode.
     * @param y the y value to set the selection's y-coordinate to.
     * @return void
     */
    void setY(qint32 y);

public:

    /**
     * gets an approximation of where the bounds on actual data 
     * are in this layer. Returns a new QRect if the Adjustment
     * Layer is neither of selection or image type.  
     * @return an approximated rectangle of this layer's outer
     * boundaries.
     */
    QRect extent() const;

    /**
     * returns the exact bounds of where the actual data resides
     * in this layer. Returns a new QRect if the AdjustmentLayer
     * is neigher of selection or image type.
     * @return an exact reclangle of this layer's outer boundaries.
     */
    QRect exactBounds() const;

    /**
     * uses the visit function which will return zero if not 
     * implemented in some child of KisNodeVisitor. The only 
     * current(2008-12-22) implementation can be found in 
     * KisLayerMapVisitor, and actually calls visitLeafNodeLayer. 
     * This means that currently the function only returns true 
     * if the AdjustmentLayer's parent is contained in a KisLayerMap.
     * @param v this this AdjustmentLayer 
     * @return true if this AdjustmentLayer's parent is contained
     * in a KisLayerMap. Otherwise returns zero.
     */
    bool accept(KisNodeVisitor &);

    /**
     * resets the cache by setting the cachedPaintDevice to a new
     * instance of KisPaintDevice. 
     * @return void
     */
    void resetCache();

    /**
     * gets this AdjustmentLayer's cached paint device
     * @return this AdjustmentLayer's KisPaintDeviceSP
     */
    KisPaintDeviceSP cachedPaintDevice();

    /**
     * gets the state of the selection - if it is shown or
     * not.
     * @return the state of the selection
     */
    bool showSelection() const;

    /**
     * sets the state of the selection to show or !show
     * @param b the state to set the selection visibility to
     * @return void
     */
    void setSelection(bool b);

    /**
     * copies the image and reformats it to thumbnail size
     * and returns the new thumbnail image. 
     * @param w width of the thumbnail to create
     * @param h height of the thumbnail to create
     * @return the thumbnail image created.
     */
    QImage createThumbnail(qint32 w, qint32 h);

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

