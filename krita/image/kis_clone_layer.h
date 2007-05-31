/*
 *  Copyright (c) 2007 Boudewijn Rempt
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
#ifndef KIS_COPY_LAYER_H_
#define KIS_COPY_LAYER_H_

#include <QObject>
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer_visitor.h"
#include "kis_layer.h"
#include "KoCompositeOp.h"

#include <krita_export.h>

class K3NamedCommand;
class QPainter;
class KisGroupLayer;
class KisFilterConfiguration;


enum CopyLayerType
{
    COPY_PROJECTION,
    COPY_ORIGINAL
};

/**
 * A copy layer adds the contents of another layer in another place in
 * the layer stack. If is possible to add more effect masks to the
 * copy. You can either copy the original data or the projection data
 * produced by the original layer + original effect masks. There is no
 * physical copy of the data; if the original changes, the copy
 * changes too. The copy layer can be positioned differently from the
 * original layer.
 **/
class KRITAIMAGE_EXPORT KisCloneLayer : public KisLayer, public KisIndirectPaintingSupport
{

public:

    KisCloneLayer(KisLayerSP from, KisImageSP img, const QString &name, quint8 opacity);
    KisCloneLayer(const KisCloneLayer& rhs);
    virtual ~KisCloneLayer();

    void updateProjection(const QRect& r);
    KisPaintDeviceSP projection() const;
    KisPaintDeviceSP paintDevice() const;

    QIcon icon() const;
    KoDocumentSectionModel::PropertyList properties() const;

    /// Return a copy of this layer
    KisLayerSP clone() const;

    qint32 x() const;
    void setX(qint32);

    qint32 y() const;
    void setY(qint32);

    /// Returns an approximation of where the bounds on actual data are in this layer
    QRect extent() const;

    /// Returns the exact bounds of where the actual data resides in this layer
    QRect exactBounds() const;

    bool accept(KisLayerVisitor &);

    QImage createThumbnail(qint32 w, qint32 h);

    // KisIndirectPaintingSupport
    KisLayer* layer() {
        return this;
    };

    void setCopyFrom( KisLayerSP layer, CopyLayerType type );
    KisLayerSP copyFrom() const;

    void setCopyType( CopyLayerType type );
    CopyLayerType copyType() const;

private:

    class Private;
    Private * m_d;

};

#endif // KIS_COPY_LAYER_H_

