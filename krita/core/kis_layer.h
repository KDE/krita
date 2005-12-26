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
class KisGroupLayer;

/**
 * Abstract class that represents the concept of a Layer in Krita. This is not related
 * to the paint devices: this is merely an abstraction of how layers can be stacked and
 * rendered differently.
 * Regarding the previous-, first-, next- and lastChild() calls, first means that it the layer
 * is at the top of the group in the layerlist, using next will iterate to the bottom to last,
 * whereas previous will go up to first again.
 **/
class KRITACORE_EXPORT KisLayer : public QObject, public KShared
{
    Q_OBJECT

public:
    KisLayer(KisImage *img, const QString &name, Q_UINT8 opacity);
    KisLayer(const KisLayer& rhs);
    virtual ~KisLayer();

    /// Return a copy of this layer
    virtual KisLayerSP clone() const = 0;

    /**
     * Returns the parent layer of a layer. This is 0 only for a root layer; otherwise
     * this will be an actual GroupLayer */
    virtual KisGroupLayerSP parent() const;
    /**
     * Returns the previous sibling of this layer in the parent's list. 0 is returned
     * if there is no parent, or if this child has no more previous siblings (== firstChild())*/
    virtual KisLayerSP prevSibling() const;
    /**
     * Returns the next sibling of this layer in the parent's list. 0 is returned
     * if there is no parent, or if this child has no more next siblings (== lastChild())*/
    virtual KisLayerSP nextSibling() const;
    /// Returns the first child layer of this layer (if it supports that).
    virtual KisLayerSP firstChild() const { return 0; }
    /// Returns the last child layer of this layer (if it supports that).
    virtual KisLayerSP lastChild() const { return 0; }

    /**
     * Returns the 'index' of this layer in the parent. This is mostly for speed in the internal
     * implementations of layer methods, so you're not really supposed to use this, or it's
     * accompanying setIndex(int). Return value is undefined when it's not a child of any layer
     */
    virtual int index() const { return m_index; }
    /**
     * Sets the 'index' of this layer. See index() for a warning and more explanations.
     * This function must be called every time this layer is added to a layer as a child!
     */
    virtual void setIndex(int index) { m_index = index; }

public:
    /// Called when the layer is made active
    virtual void activate() {};

    /// Called when another layer is made active
    virtual void deactivate() {};

public:
    virtual Q_INT32 x() const = 0;
    virtual void setX(Q_INT32) = 0;

    virtual Q_INT32 y() const = 0;
    virtual void setY(Q_INT32) = 0;

    /// Returns an approximation of where the bounds on actual data are in this layer
    virtual QRect extent() const = 0;
    /// Returns the exact bounds of where the actual data resides in this layer
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

    /// paints a mask where the selection on this layer resides
    virtual void paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    /// paints where no data is on this layer. Useful when it is a transparent layer stacked on top of another one
    virtual void paintMaskInactiveLayers(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

    /// Accept the KisLayerVisitor (for the Visitor design pattern), should call the correct function on the KisLayerVisitor for this layer type
    virtual void accept(KisLayerVisitor &) = 0;

    void setParent(KisGroupLayerSP parent);

signals:
    void visibilityChanged(KisLayerSP device);

private:
    Q_UINT8 m_opacity;
    bool m_linked;
    bool m_locked;
    bool m_visible;
    QString m_name;
    KisGroupLayerSP m_parent;
    KisImage *m_image;
    int m_index;

    // Operation used to composite this layer with the layers _under_ this layer
    KisCompositeOp m_compositeOp;
};

#endif // KIS_LAYER_H_

