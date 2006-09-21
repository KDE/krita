/*
 *  Copyright (c) 2006 Boudewijn Rempt
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
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

class KNamedCommand;
class QPainter;
class KisUndoAdapter;
class KisGroupLayer;
class KisFilterConfiguration;

/**
 * Class that contains a KisFilter and optionally a KisSelection. The combination
 * is used by to influence the rendering of the layers under this layer in the
 * layerstack
 **/
class KRITAIMAGE_EXPORT KisAdjustmentLayer : public KisLayer
{
    typedef KisLayer super;
    Q_OBJECT

public:
    /**
     * Create a new adjustment layer with the given configuration and selection.
     * Note that the selection will be _copied_.
     */
    KisAdjustmentLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection);
    KisAdjustmentLayer(const KisAdjustmentLayer& rhs);
    virtual ~KisAdjustmentLayer();

    virtual QIcon icon() const;
    virtual PropertyList properties() const;

    /// Return a copy of this layer
    virtual KisLayerSP clone() const;

public:

    KisFilterConfiguration * filter() const;
    void setFilter(KisFilterConfiguration * filterConfig);

    KisSelectionSP selection();

    /// Set the selction of this adjustment layer to a copy of selection.
    void setSelection(KisSelectionSP selection);

public:

    virtual qint32 x() const;
    virtual void setX(qint32);

    virtual qint32 y() const;
    virtual void setY(qint32);

    /// Returns an approximation of where the bounds on actual data are in this layer
    virtual QRect extent() const;

    /// Returns the exact bounds of where the actual data resides in this layer
    virtual QRect exactBounds() const;

    virtual bool accept(KisLayerVisitor &);

    virtual void resetCache();
    virtual KisPaintDeviceSP cachedPaintDevice() { return m_cachedPaintDev; }

private:

    KisFilterConfiguration * m_filterConfig;
    KisSelectionSP m_selection;
    KisPaintDeviceSP m_cachedPaintDev;
};

#endif // KIS_ADJUSTMENT_LAYER_H_

