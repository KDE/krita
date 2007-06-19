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
#include "kis_layer_visitor.h"
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

    void updateProjection(const QRect& r);
    KisPaintDeviceSP projection() const;
    KisPaintDeviceSP paintDevice() const;

    QIcon icon() const;
    KoDocumentSectionModel::PropertyList properties() const;

    /// Return a copy of this layer
    KisLayerSP clone() const;

public:

    KisFilterConfiguration * filter() const;
    void setFilter(KisFilterConfiguration * filterConfig);

    KisSelectionSP selection();

    /// Set the selection of this adjustment layer to a copy of selection.
    void setSelection(KisSelectionSP selection);

public:

    qint32 x() const;
    void setX(qint32);

    qint32 y() const;
    void setY(qint32);

    /// Returns an approximation of where the bounds on actual data are in this layer
    QRect extent() const;

    /// Returns the exact bounds of where the actual data resides in this layer
    QRect exactBounds() const;

    bool accept(KisLayerVisitor &);

    void resetCache();
    KisPaintDeviceSP cachedPaintDevice() { return m_cachedPaintDev; }

    bool showSelection() const { return m_showSelection; }
    void setSelection(bool b) { m_showSelection = b; }

    QImage createThumbnail(qint32 w, qint32 h);

    // KisIndirectPaintingSupport
    KisLayer* layer() { return this; }

private:
    bool m_showSelection;
    KisFilterConfiguration * m_filterConfig;
    KisSelectionSP m_selection;
    KisSelectionSP m_selectionProjection;
    KisPaintDeviceSP m_cachedPaintDev;
    QRegion m_dirtyRegion;


};

#endif // KIS_ADJUSTMENT_LAYER_H_

