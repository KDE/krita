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
#include <krita_export.h>
#include "kis_node_filter_interface.h"

class KisNodeVisitor;

/**
 * @class KisSelectionBasedLayer describes base behaviour for
 * selection base classes like KisAdjustmentLayer and KisGeneratorLayer.
 * These clesses should have a persistent selection that controls
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
    KisSelectionBasedLayer(KisImageWSP image, const QString &name, KisSelectionSP selection);
    KisSelectionBasedLayer(const KisSelectionBasedLayer& rhs);
    virtual ~KisSelectionBasedLayer();


    /**
     * tells whether the @node can be a child of this layer
     * @param node to be connected node
     * @return tells if to be connected is a child of KisMask
     */
    bool allowAsChild(KisNodeSP node) const;

    KisPaintDeviceSP original() const;
    KisPaintDeviceSP paintDevice() const;

    QRect repaintOriginal(KisPaintDeviceSP original,
                          const QRect& rect);

    bool needProjection() const;
    void copyOriginalToProjection(const KisPaintDeviceSP original,
                                  KisPaintDeviceSP projection,
                                  const QRect& rect) const;

    // From KisNode
    QRect changeRect(const QRect &rect) const;
    QRect needRect(const QRect &rect) const;

    /**
     * resets cached projection of lower layer to a new device
     * @return void
     */
    void resetCache(const KoColorSpace *colorSpace = 0);

    /**
     * for KisLayer::setDirty() and KisLayer::setDirty(const QRegion&)
     */
    using KisLayer::setDirty;

    /**
     * Mark a layer as dirty. We can't use KisLayer's one
     * as our extent() function doesn't fit for this
     */
    void setDirty();

public:

    /**
     * gets this layer's selection
     * @return the lsyer's selection
     */
    KisSelectionSP selection() const;

    /**
     * sets the selection of this layer to a copy of
     * selection
     * @param selection the selection to set
     * @return void
     */
    void setSelection(KisSelectionSP selection);

    /**
     * gets the state of the selection - if it is shown or
     * not.
     * @return the state of the selection
     */
    bool showSelection() const;

    /**
     * sets the state of the selection to show or !show
     * @param show the state to set the selection visibility to
     * @return void
     */
    void setShowSelection(bool show);

    /**
     * gets this layer's x coordinate, taking selection into account
     * @return x-coordinate value
     */
    qint32 x() const;

    /**
     * gets this layer's y coordinate, taking selection into account
     * @return y-coordinate value
     */
    qint32 y() const;

    /**
     * sets this layer's y coordinate, taking selection into account
     * @param x x coordinate
     */
    void setX(qint32 x);

    /**
     * sets this layer's y coordinate, taking selection into account
     * @param y y coordinate
     */
    void setY(qint32 y);

public:

    /**
     * gets an approximation of where the bounds on actual data
     * are in this layer, taking selection into account
     */
    QRect extent() const;

    /**
     * returns the exact bounds of where the actual data resides
     * in this layer, taking selection into account
     */
    QRect exactBounds() const;

    /**
     * copies the image and reformats it to thumbnail size
     * and returns the new thumbnail image.
     * @param w width of the thumbnail to create
     * @param h height of the thumbnail to create
     * @return the thumbnail image created.
     */
    QImage createThumbnail(qint32 w, qint32 h);

protected:
    void initSelection();

public slots:

    /**
     * gets this layer. Overriddes function in
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

#endif /* KIS_SELECTION_BASED_LAYER_H_ */

