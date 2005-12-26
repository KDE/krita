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
 * Internally, the layers are ordered like this: first layer = top in layerbox = list.end()
 * the index() calls of children will return values in accordance to this
 * (firstChild -> index = m_layers.count() - 1; lastChild -> index = 0)
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

    virtual QRect extent() const {return QRect();};
    virtual QRect exactBounds() const {return QRect();};

    virtual void accept(KisLayerVisitor &v) { v.visit(this); }

    virtual KisLayerSP firstChild() const;
    virtual KisLayerSP lastChild() const;

    /**
     * Add the specified layer above the specified layer (if aboveThis == 0, or aboveThis is no
     * child of this layer, the bottom is used)
     * Returns false if the layer already is a direct child of this group */
    bool addLayer(KisLayerSP newLayer, KisLayerSP aboveThis);
    /// Remove the layer from this group. If the layer is no child of this one, false is returned
    bool removeLayer(KisLayerSP layer);

    /// returns the previous sibling of the child layer specified, conforming to what prevSibling expects (returns 0 if it is not a child)
    virtual KisLayerSP prevSiblingOf(const KisLayer* layer) const;
    /// returns the next sibling of the child layer specified, conforming to what nextSibling expects (returns 0 if it is not a child)
    virtual KisLayerSP nextSiblingOf(const KisLayer* layer) const;

private:
    vKisLayerSP m_layers; // Contains the list of all layers
};

#endif // KIS_GROUP_LAYER_H_

