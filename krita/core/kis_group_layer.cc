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

#include <kdebug.h>
#include <qimage.h>

#include "kis_debug_areas.h"
#include "kis_group_layer.h"

KisGroupLayer::KisGroupLayer(KisImage *img, const QString &name, Q_UINT8 opacity) :
    super(img, name, opacity)
{
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) : super(rhs)
{
}

KisLayerSP KisGroupLayer::clone() const
{
    return new KisGroupLayer(*this);
}

KisGroupLayer::~KisGroupLayer()
{
}

bool KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)
{
    if (m_layers.contains(newLayer))
        return false;

    newLayer -> setParent(this);

    // We enter this loop even if aboveThis == 0, because we need to increment the layers
    // above insertion pos their indices with 1 each
    for (int layerIndex = m_layers.size() - 1; layerIndex >= 0; layerIndex--) {
        KisLayerSP layer = m_layers[layerIndex];
        if (layer == aboveThis)
        {
            // begin() + insertPosition -> insert BEFORE that (hence the + 1)
            m_layers.insert(m_layers.begin() + (layerIndex + 1), newLayer);
            newLayer -> setIndex(layerIndex + 1); // This is now the layer right above index
            return true;
        } else {
            layer -> setIndex(layer -> index() + 1);
        }
    }
    // Falls through to be added on bottom

    //Add to bottom
    m_layers.insert(m_layers.begin(), newLayer);
    newLayer -> setIndex(0);

    return true;
}

bool KisGroupLayer::removeLayer(KisLayerSP layer)
{
    if (!m_layers.contains(layer))
        return false; // because we'll decrease indices incorrectly otherwise!

    for (int layerIndex = m_layers.size() - 1; layerIndex >= 0; layerIndex--) {
        KisLayerSP current = m_layers[layerIndex];
        if (current == layer)
        {
            m_layers.erase(m_layers.begin() + layerIndex);
            layer->setParent(0);
            return true;
        } else {
            current -> setIndex(layer -> index() - 1);
        }
    }

    return false;
}

KisLayerSP KisGroupLayer::firstChild() const {
    if (m_layers.count() == 0) {
        kdDebug() << "No children for firstChild!" << endl;
        return 0;
    }

    return m_layers.at(m_layers.count() - 1);
}

KisLayerSP KisGroupLayer::lastChild() const {
    if (m_layers.count() == 0) {
        kdDebug() << "No children for lastChild!" << endl;
        return 0;
    }

    return m_layers.at(0);
}

KisLayerSP KisGroupLayer::prevSiblingOf(const KisLayer* layer) const {
    if (layer -> parent() != this)
        return 0;

    // previous sibling -> up in the layerbox -> index + 1
    int index = layer -> index();
    if (index == m_layers.count() - 1)
        return 0;
    return m_layers.at(index + 1);
}

KisLayerSP KisGroupLayer::nextSiblingOf(const KisLayer* layer) const {
    if (layer -> parent() != this)
        return 0;

    // next sibling -> down in the layerbox -> index - 1
    int index = layer -> index();
    if (index == 0)
        return 0;
    return m_layers.at(index - 1);
}

#include "kis_group_layer.moc"
