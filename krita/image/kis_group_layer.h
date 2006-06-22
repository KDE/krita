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
#ifndef KIS_GROUP_LAYER_H_
#define KIS_GROUP_LAYER_H_

#include <ksharedptr.h>

#include "kis_layer.h"
#include "kis_types.h"

#include "kis_paint_layer.h"

class KisMergeVisitor;

/**
 * A KisLayer that bundles child layers into a single layer.
 * The top layer is firstChild(), with index 0; the bottommost lastChild() with index childCount() - 1.
 * KisLayer::nextSibling() moves towards higher indices, from the top to the bottom layer; prevSibling() the reverse.
 * (Implementation detail: internally, the indices are reversed, for speed.)
 **/
class KRITAIMAGE_EXPORT KisGroupLayer : public KisLayer {
    typedef KisLayer super;

    Q_OBJECT

public:
    KisGroupLayer(KisImage *img, const QString &name, quint8 opacity);
    KisGroupLayer(const KisGroupLayer& rhs);
    virtual ~KisGroupLayer();

    virtual QIcon icon() const;

    virtual KisLayerSP clone() const;
public:

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

    virtual void activate() {};

    virtual void deactivate() {};

    virtual qint32 x() const;
    virtual void setX(qint32);

    virtual qint32 y() const;
    virtual void setY(qint32);

    // Sets this layer and all its descendants' owner image to the given image.
    virtual void setImage(KisImage *image);

    virtual QRect extent() const;
    virtual QRect exactBounds() const;

    virtual bool accept(KisLayerVisitor &v)
        {
//            kDebug(41001) << "GROUP\t\t" << name()
//                    << " dirty: " << dirty()
//                    << ", " << m_layers.count() << " children "
//                    << ", projection: " << m_projection
//                    << "\n";
            return v.visit(this);
        };

    virtual void resetProjection();
    virtual KisPaintDeviceSP projection(const QRect & rect);

    virtual uint childCount() const;

    virtual KisLayerSP firstChild() const;
    virtual KisLayerSP lastChild() const;

    /// Returns the layer at the specified index.
    virtual KisLayerSP at(int index) const;

    /// Returns the index of the layer if it's in this group, or -1 otherwise.
    virtual int index(KisLayerSP layer) const;

    /// Moves the specified layer to the specified index in the group, if it's already a member of this group.
    virtual void setIndex(KisLayerSP layer, int index);

    /** Adds the layer to this group at the specified index. childCount() is a valid index and appends to the end.
        Fails and returns false if the layer is already in this group or any other (remove it first.) */
    virtual bool addLayer(KisLayerSP newLayer, int index);

    /**
     * Add the specified layer above the specified layer (if aboveThis == 0, the bottom is used) */
    virtual bool addLayer(KisLayerSP newLayer, KisLayerSP aboveThis);

    /// Removes the layer at the specified index from the group.
    virtual bool removeLayer(int index);

    /// Removes the layer from this group. Fails if there's no such layer in this group.
    virtual bool removeLayer(KisLayerSP layer);

    virtual QImage createThumbnail(qint32 w, qint32 h);

signals:

    void sigDirty(QRect rc);

public: // from QAbstractItemModel

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:

    void updateProjection(const QRect & rc);

    inline int reverseIndex(int index) const { return childCount() - 1 - index; };
    vKisLayerSP m_layers; // Contains the list of all layers
    KisPaintDeviceSP m_projection; // The cached composition of all layers in this group

    qint32 m_x;
    qint32 m_y;
};

#endif // KIS_GROUP_LAYER_H_

