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

#include "kis_layer.h"
#include "kis_types.h"


/**
 * A KisLayer that bundles child layers into a single layer.
 * The top layer is firstChild(), with index 0; the bottommost lastChild() with index childCount() - 1.
 * KisLayer::nextSibling() moves towards higher indices, from the top to the bottom layer; prevSibling() the reverse.
 * (Implementation detail: internally, the indices are reversed, for speed.)
 **/
class KisGroupLayer : public KisLayer {
    typedef KisLayer super;

    Q_OBJECT

public:
    KisGroupLayer(KisImage *img, const QString &name, Q_UINT8 opacity);
    KisGroupLayer(const KisGroupLayer& rhs);
    virtual ~KisGroupLayer();

    virtual KisLayerSP clone() const;
public:

    virtual void activate() {};

    virtual void deactivate() {};

    virtual Q_INT32 x() const {return 0;};
    virtual void setX(Q_INT32) {};

    virtual Q_INT32 y() const {return 0;};
    virtual void setY(Q_INT32) {};

    // Sets this layer and all its descendants' owner image to the given image.
    virtual void setImage(KisImage *image);

    virtual QRect extent() const;
    virtual QRect exactBounds() const;

    virtual bool accept(KisLayerVisitor &v)
        {
            kdDebug(41001) << "Visiting on group layer " << name()
                    << ", " << m_layers.count() << " children "
                    << ", projection: " << m_projection << "\n";
            return v.visit(this);
        };

    virtual KisPaintDeviceImplSP projection() { return m_projection; }

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

private:
    inline int reverseIndex(int index) const { return childCount() - 1 - index; };
    vKisLayerSP m_layers; // Contains the list of all layers
    KisPaintDeviceImplSP m_projection; // The cached composition of all layers in this group
};

#endif // KIS_GROUP_LAYER_H_

