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

#include <QRect>

#include "krita_export.h"

#include "kis_types.h"
#include "kis_layer_visitor.h"
#include "KoCompositeOp.h"
#include "KoDocumentSectionModel.h"

class KNamedCommand;
class QIcon;
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
class KRITAIMAGE_EXPORT KisLayer: public KoDocumentSectionModel, public KShared
{
    typedef KoDocumentSectionModel super;
    Q_OBJECT

public:
    KisLayer(KisImage *img, const QString &name, quint8 opacity);
    KisLayer(const KisLayer& rhs);
    virtual ~KisLayer();

    virtual QIcon icon() const = 0;
    virtual PropertyList properties() const;

    /**
     * Set the specified rect to clean
     */
    virtual void setClean(const QRect & rect);

    /**
     * If the layer has been changed and not been composited yet, this returns true
     */
    virtual bool dirty();

    /**
     * Return true if the given rect intersects the dirty rect(s) of this layer
     */
    virtual bool dirty(const QRect & rc);


    virtual QRect dirtyRect() const;


    /**
     * Set the entire layer extent dirty; this percolates up to parent layers all the
     * way to the root layer.
     */
    virtual void setDirty(bool propagate = true);

    /**
     * Add the given rect to the set of dirty rects for this layer;
     * this percolates up to parent layers all the way to the root
     * layer.
     */
    virtual void setDirty(const QRect & rect, bool propagate = true);

    /// Return a copy of this layer
    virtual KisLayerSP clone() const = 0;

    /// Returns the ID of the layer, which is guaranteed to be unique among all KisLayers.
    int id() const { return m_id; }

    /* Returns the index of the layer in its parent's list of child layers. Indices
     * increase from 0, which is the topmost layer in the list, to the bottommost.
     */
    virtual int index() const;

    /// Moves this layer to the specified index within its parent's list of child layers.
    virtual void setIndex(int index);

    /**
     * Returns the parent layer of a layer. This is 0 only for a root layer; otherwise
     * this will be an actual GroupLayer */
    virtual KisGroupLayerSP parent() const;

    /**
     * Returns the previous sibling of this layer in the parent's list. This is the layer
     * *above* this layer. 0 is returned if there is no parent, or if this child has no more
     * previous siblings (== firstChild())
     */
    virtual KisLayerSP prevSibling() const;

    /**
     * Returns the next sibling of this layer in the parent's list. This is the layer *below*
     * this layer. 0 is returned if there is no parent, or if this child has no more next
     * siblings (== lastChild())
     */
    virtual KisLayerSP nextSibling() const;

    /**
     * Returns the sibling above this layer in its parent's list. 0 is returned if there is no parent,
     * or if this layer is the topmost layer in its group. This is the same as calling prevSibling().
     */
    KisLayerSP siblingAbove() const { return prevSibling(); }

    /**
     * Returns the sibling below this layer in its parent's list. 0 is returned if there is no parent,
     * or if this layer is the bottommost layer in its group.  This is the same as calling nextSibling().
     */
    KisLayerSP siblingBelow() const { return nextSibling(); }

    /// Returns how many direct child layers this layer has (not recursive).
    virtual uint childCount() const { return 0; }

    virtual KisLayerSP at(int /*index*/) const { return KisLayerSP(0); }

    /// Returns the first child layer of this layer (if it supports that).
    virtual KisLayerSP firstChild() const { return KisLayerSP(0); }

    /// Returns the last child layer of this layer (if it supports that).
    virtual KisLayerSP lastChild() const { return KisLayerSP(0); }

    /// Recursively searches this layer and any child layers for a layer with the specified name.
    virtual KisLayerSP findLayer(const QString& name) const;

    /// Recursively searches this layer and any child layers for a layer with the specified ID.
    virtual KisLayerSP findLayer(int id) const;

    enum { Visible = 1, Hidden = 2, Locked = 4, Unlocked = 8 };

    /// Returns the total number of layers in this layer, its child layers, and their child layers recursively, optionally ones with the specified properties Visible or Locked, which you can OR together.
    virtual int numLayers(int type = 0) const;

public:
    /// Called when the layer is made active
    virtual void activate() {};

    /// Called when another layer is made active
    virtual void deactivate() {};

public:
    virtual qint32 x() const = 0;
    virtual void setX(qint32) = 0;

    virtual qint32 y() const = 0;
    virtual void setY(qint32) = 0;

    virtual KNamedCommand *moveCommand(QPoint oldPosition, QPoint newPosition);

    /// Returns an approximation of where the bounds on actual data are in this layer
    virtual QRect extent() const = 0;
    /// Returns the exact bounds of where the actual data resides in this layer
    virtual QRect exactBounds() const = 0;

    virtual const bool visible() const;
    virtual void setVisible(bool v);
    KNamedCommand *setVisibleCommand(bool visiblel);

    quint8 opacity() const; //0-255
    void setOpacity(quint8 val); //0-255
    quint8 percentOpacity() const; //0-100
    void setPercentOpacity(quint8 val); //0-100
    KNamedCommand *setOpacityCommand(quint8 val);
    KNamedCommand *setOpacityCommand(quint8 prevOpacity, quint8 newOpacity);

    bool locked() const;
    void setLocked(bool l);
    KNamedCommand *setLockedCommand(bool locked);

    void notifyPropertyChanged();
    void notifyCommandExecuted();

    bool temporary() const;
    void setTemporary(bool t);

    virtual QString name() const;
    virtual void setName(const QString& name);

    KoCompositeOp compositeOp() const { return m_compositeOp; }
    void setCompositeOp(const KoCompositeOp& compositeOp);
    KNamedCommand *setCompositeOpCommand(const KoCompositeOp& compositeOp);

    KisImage *image() const { return m_image; }
    virtual void setImage(KisImage *image) { m_image = image; }

    KisUndoAdapter *undoAdapter() const;

    /// paints a mask where the selection on this layer resides
    virtual void paintSelection(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h);
    virtual void paintSelection(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize);

    /// paints where no data is on this layer. Useful when it is a transparent layer stacked on top of another one
    virtual void paintMaskInactiveLayers(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h);

    /// Returns a thumbnail in requested size. The QImage may have transparent parts.
    /// May also return 0
    virtual QImage createThumbnail(qint32 w, qint32 h);

    /// Accept the KisLayerVisitor (for the Visitor design pattern), should call the correct function on the KisLayerVisitor for this layer type
    virtual bool accept(KisLayerVisitor &) = 0;

public: // from QAbstractItemModel
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    QModelIndex indexFromLayer(KisLayer *layer) const;

    /// causes QAbstractItemModel::dataChanged() to be emitted and percolated up the tree
    void notifyPropertyChanged(KisLayer *layer);

private:
    friend class KisGroupLayer;

    bool matchesFlags(int flags) const;

    int m_id;
    int m_index;
    quint8 m_opacity;
    bool m_locked;
    bool m_visible;
    bool m_temporary;

    // XXX: keep a list of dirty rects instead of always aggegrating them
    QRect m_dirtyRect;
    QString m_name;
    KisGroupLayerSP m_parent;
    KisImage *m_image;

    // Operation used to composite this layer with the layers _under_ this layer
    KoCompositeOp m_compositeOp;
};

#endif // KIS_LAYER_H_

