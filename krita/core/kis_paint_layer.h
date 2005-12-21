/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_PAINT_LAYER_H_
#define KIS_PAINT_LAYER_H_

#include "kis_paint_device_impl.h"
#include "kis_layer.h"
#include "kis_types.h"


class KisPaintLayer : public KisLayer {
    typedef KisLayer super;

    Q_OBJECT

public:
    KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity);
    KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisColorSpace * colorSpace);
    KisPaintLayer(const KisPaintLayer& rhs);
    virtual ~KisPaintLayer();

    virtual KisLayerSP clone() const;
public:

    // Called when the layer is made active
    virtual void activate() {};

    // Called when another layer is made active
    virtual void deactivate() {};

    virtual Q_INT32 x() const { return m_paintdev->getX(); };
    virtual void setX(Q_INT32 x) { m_paintdev->setX(x); };

    virtual Q_INT32 y() const { return m_paintdev->getY(); };
    virtual void setY(Q_INT32 y) { m_paintdev->setY(y); };

    virtual QRect extent() const {return m_paintdev->extent(); };
    virtual QRect exactBounds() const {return m_paintdev->exactBounds(); };

    virtual void paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    virtual void paintMaskInactiveLayers(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

    virtual void accept(KisLayerVisitor &v) { v.visit(this); };

    KisPaintDeviceImplSP paintDevice() { return m_paintdev; };

private:
    KisPaintDeviceImplSP m_paintdev;
};

#endif // KIS_PAINT_LAYER_H_

