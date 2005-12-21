/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_LAYER_H_
#define KIS_LAYER_H_

#include <qobject.h>
#include "kis_types.h"
#include "kis_layer_visitor.h"
#include "kis_composite_op.h"
#include <koffice_export.h>

class KNamedCommand;
class QPainter;
class KisUndoAdapter;

class KRITACORE_EXPORT KisLayer : public QObject, public KShared
{
    Q_OBJECT

public:
    KisLayer(KisImage *img, const QString &name, Q_UINT8 opacity);
    KisLayer(const KisLayer& rhs);
    virtual ~KisLayer();

    virtual KisLayerSP clone() const = 0;

    virtual KisLayerSP parent() const;
    virtual KisLayerSP prevSibling() const{return 0;};
    virtual KisLayerSP nextSibling() const{return 0;};
    virtual KisLayerSP firstChild() const{return 0;};
    virtual KisLayerSP lastChild() const{return 0;};

public:
    // Called when the layer is made active
    virtual void activate() {};

    // Called when another layer is made active
    virtual void deactivate() {};

public:
    virtual Q_INT32 x() const = 0;
    virtual void setX(Q_INT32) = 0;

    virtual Q_INT32 y() const = 0;
    virtual void setY(Q_INT32) = 0;

    virtual QRect extent() const = 0;
    virtual QRect exactBounds() const = 0;

    virtual const bool visible() const;
    virtual void setVisible(bool v);
    KNamedCommand *setVisibleCommand(bool visiblel);

    Q_UINT8 opacity() const;
    void setOpacity(Q_UINT8 val);
    KNamedCommand *setOpacityCommand(Q_UINT8 val);

    bool linked() const;
    void setLinked(bool l);
    KNamedCommand *setLinkedCommand(bool linked);

    bool locked() const;
    void setLocked(bool l);
    KNamedCommand *setLockedCommand(bool locked);

    virtual QString name() const;
    virtual void setName(const QString& name);

    KisCompositeOp compositeOp() { return m_compositeOp; }
    void setCompositeOp(const KisCompositeOp& compositeOp) { m_compositeOp = compositeOp; }
    KNamedCommand *setCompositeOpCommand(const KisCompositeOp& compositeOp);

    KisImage *image() { return m_image; };

    KisUndoAdapter *undoAdapter() const;

    virtual void paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    virtual void paintMaskInactiveLayers(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

    virtual void insertLayer(KisLayerSP newLayer, KisLayerSP belowLayer);
    virtual void removeLayer(KisLayerSP layer);

    virtual void accept(KisLayerVisitor &) = 0;

    void setParent(KisLayerSP parent) { m_parent=parent;};

signals:
    void visibilityChanged(KisLayerSP device);

private:
    Q_UINT8 m_opacity;
    bool m_linked;
    bool m_locked;
    bool m_visible;
    QString m_name;
    KisLayerSP m_parent;
    KisImage *m_image;

    // Operation used to composite this layer with the layers _under_ this layer
    KisCompositeOp m_compositeOp;
};

#endif // KIS_LAYER_H_

