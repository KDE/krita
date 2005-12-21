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

void KisGroupLayer::insertLayer(KisLayerSP newLayer, KisLayerSP aboveThis)
{
    if(!aboveThis)
    {
        for (int layerIndex = m_layers.size() - 1; layerIndex >= 0; layerIndex--) {
            if (m_layers[layerIndex] == aboveThis)
            {
                m_layers.insert(m_layers.begin() + layerIndex, newLayer);
                newLayer->setParent(this);
                return;
            }
        }
        // Falls through to be added on bottom
    }

    //Add to bottom
    m_layers.push_back(newLayer);
}

void KisGroupLayer::removeLayer(KisLayerSP layer)
{
    for (int layerIndex = m_layers.size() - 1; layerIndex >= 0; layerIndex--) {
        if (m_layers[layerIndex] == layer)
        {
            m_layers.erase(m_layers.begin() + layerIndex);
            layer->setParent(0);
            return;
        }
    }
}

#include "kis_group_layer.moc"
