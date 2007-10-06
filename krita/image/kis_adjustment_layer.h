/*
 *  Copyright (c) 2006 Boudewijn Rempt
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
#ifndef KIS_ADJUSTMENT_LAYER_H_
#define KIS_ADJUSTMENT_LAYER_H_

#include <QObject>
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_node_visitor.h"
#include "kis_layer.h"
#include "KoCompositeOp.h"
#include <krita_export.h>

class KisFilterConfiguration;

/**
 * Class that contains a KisFilter and optionally a KisSelection. The combination
 * is used by to influence the rendering of the layers under this layer in the
 * layerstack.
 *
 * AdjustmentLayers also function as a kind of "fixating layers".
 *
 * XXX_NODE: implement prepareForRemoval with:
         // Adjustment layers should mark the layers underneath them, whose rendering
        // they have cached, dirty on removal. Otherwise, the group won't be re-rendered.
        KisAdjustmentLayer * al = dynamic_cast<KisAdjustmentLayer*>(layer.data());
        if (al) {
            QRect r = al->extent();
            lock(); // Lock the image, because we are going to dirty a lot of layers
            KisLayerSP l = layer->nextSibling();
            while (l) {
                KisAdjustmentLayer * al2 = dynamic_cast<KisAdjustmentLayer*>(l.data());
                if (al2 != 0) break;
                l = l->nextSibling();
            }
            unlock();
        }

 */
class KRITAIMAGE_EXPORT KisAdjustmentLayer : public KisLayer, public KisIndirectPaintingSupport
{
    Q_OBJECT

public:
    /**
     * Create a new adjustment layer with the given configuration and selection.
     * Note that the selection will be _copied_.
     */
    KisAdjustmentLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection);
    KisAdjustmentLayer(const KisAdjustmentLayer& rhs);
    virtual ~KisAdjustmentLayer();

    KisNodeSP clone()
        {
            return KisNodeSP(new KisAdjustmentLayer(*this));
        }

    bool allowAsChild( KisNodeSP );

    void updateProjection(const QRect& r);
    KisPaintDeviceSP projection() const;
    KisPaintDeviceSP paintDevice() const;

    QIcon icon() const;
    KoDocumentSectionModel::PropertyList sectionModelProperties() const;

    /// Return a copy of this layer
    KisLayerSP clone() const;

public:

    KisFilterConfiguration * filter() const;
    void setFilter(KisFilterConfiguration * filterConfig);

    KisSelectionSP selection() const;

    /// Set the selection of this adjustment layer to a copy of selection.
    void setSelection(KisSelectionSP selection);

    /**
     * overriden from KisBaseNode
     */
    qint32 x() const;

    /**
     * overriden from KisBaseNode
     */
    void setX(qint32 x);

    /**
     * overriden from KisBaseNode
     */
    qint32 y() const;

    /**
     * overriden from KisBaseNode
     */
    void setY(qint32 y);

public:

    /// Returns an approximation of where the bounds on actual data are in this layer
    QRect extent() const;

    /// Returns the exact bounds of where the actual data resides in this layer
    QRect exactBounds() const;

    bool accept(KisNodeVisitor &);

    void resetCache();

    KisPaintDeviceSP cachedPaintDevice();

    bool showSelection() const;
    void setSelection(bool b);

    QImage createThumbnail(qint32 w, qint32 h);

    // KisIndirectPaintingSupport
    KisLayer* layer()
        {
            return this;
        }

private:

    class Private;
    Private * const m_d;
};

#endif // KIS_ADJUSTMENT_LAYER_H_

