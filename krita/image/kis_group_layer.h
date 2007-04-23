/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
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
#ifndef KIS_GROUP_LAYER_H_
#define KIS_GROUP_LAYER_H_

#include <ksharedptr.h>

#include "kis_layer.h"
#include "kis_types.h"

#include "kis_paint_layer.h"

class KoColorSpace;
class KisMergeVisitor;

/**
 * A KisLayer that bundles child layers into a single layer.
 * The top layer is firstChild(), with index 0; the bottommost lastChild() with index childCount() - 1.
 * KisLayer::nextSibling() moves towards higher indices, from the top to the bottom layer; prevSibling() the reverse.
 * (Implementation detail: internally, the indices are reversed, for speed.)
 **/
class KRITAIMAGE_EXPORT KisGroupLayer : public KisLayer {

    Q_OBJECT

public:
    KisGroupLayer(KisImageSP img, const QString &name, quint8 opacity);
    KisGroupLayer(const KisGroupLayer& rhs);
    virtual ~KisGroupLayer();

    virtual QIcon icon() const;

    virtual KisLayerSP clone() const;

signals:

    /**
       Emitted whenever the specified region has been dirtied.
    */
    void sigDirtyRegionAdded( const QRegion & );

    /**
       Emitten whenver the specified rect has been dirtied.
    */
    void sigDirtyRectAdded( const QRect & );

public:

    virtual KoColorSpace * colorSpace();

    /**
     * Set the entire layer extent dirty; this percolates up to parent layers all the
     * way to the root layer.
     */
    virtual void setDirty();

    /**
     * Add the given rect to the set of dirty rects for this layer;
     * this percolates up to parent layers all the way to the root
     * layer.
     */
    virtual void setDirty(const QRect & rect);

    /**
     * Add the given region to the set of dirty rects for this layer;
     * this percolates up to parent layers all the way to the root
     * layer.
     */
    virtual void setDirty( const QRegion & region);

    virtual qint32 x() const;
    virtual void setX(qint32);

    virtual qint32 y() const;
    virtual void setY(qint32);

    /**
       Sets this layer and all its descendants' owner image to the
       given image.
    */
    virtual void setImage(KisImageSP image);

    /**
       Return the united extents of all layers in this group layer;
       this function is _recursive_.
     */
    virtual QRect extent() const;

    /**
       Return the exact bounding rect of all layers in this group
       layer; this function is _recursive_ and can therefore be really
       slow.
     */
    virtual QRect exactBounds() const;

    /**
       Accect the specified visitor.
       @return true if the operation succeeded, false if it failed.
    */
    virtual bool accept(KisLayerVisitor &v)
        {
            return v.visit(this);
        };

    /**
       Clear the projection or create a projection from the specified
       paint devide.

       Warning: will copy from to, if !0,

       Note for hackers: implement CoW!
     */
    virtual void resetProjection(KisPaintDeviceSP to = 0);

    /**
       Retrieve the projection for this group layer. Note that
       The projection is _not_ guaranteed to be up to date with
       the latest actions, and that you cannot discover whether it
       is!

       Note the second: this _may_ return the paint device of a paint
       layer if that paint layer is the only child of this group layer.
    */
    virtual KisPaintDeviceSP projection();

    /**
       Update the given rect of the projection paint device.

       Note for hackers: keep this method thread-safe!
    */
    void updateProjection(const QRect & rc);

    /**
       @return the number of layers contained in this group layer. The
       count is _not_ recursive, i.e., a child grouplayer that
       contains ten other layers counts for one.
    */
    virtual uint childCount() const;

    /**
     @return the bottom-most layer of the layers contained in this
     group. This is not recursive: if the bottom-most layer is a
     grouplayer, the grouplayer is returned, not the first child of
     that group layer.
    */
    virtual KisLayerSP firstChild() const;

    /**
     @return the top-most layer of the layers contained in this
     group. This is not recursive: if the top-most layer is a
     grouplayer, the grouplayer is returned, not the last child
     of that group layer.
    */
    virtual KisLayerSP lastChild() const;

    /**
       @returns the layer at the specified index.
    */
    virtual KisLayerSP at(int index) const;

    /**
       @returns the index of the specified layer if it's in this
       group, or -1 otherwise.
    */
    virtual int index(KisLayerSP layer) const;

    virtual QImage createThumbnail(qint32 w, qint32 h);

    /// Returns if the layer will induce the projection hack (if the only layer in this group)
    virtual bool paintLayerInducesProjectionOptimization(KisPaintLayerSP l);

private:

    friend class KisImage; // Only KisImage is allowed to add layers

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

private:

    inline int reverseIndex(int index) const { return childCount() - 1 - index; };
    vKisLayerSP m_layers; // Contains the list of all layers
    KisPaintDeviceSP m_projection; // The cached composition of all layers in this group

    qint32 m_x;
    qint32 m_y;
};

#endif // KIS_GROUP_LAYER_H_

