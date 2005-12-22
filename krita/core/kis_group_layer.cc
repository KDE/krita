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

    if(aboveThis)
    {
        for (int layerIndex = m_layers.size() - 1; layerIndex >= 0; layerIndex--) {
            if (m_layers[layerIndex] == aboveThis)
            {
                m_layers.insert(m_layers.begin() + layerIndex, newLayer);
                newLayer->setParent(this);
                return true;
            }
        }
        // Falls through to be added on bottom
    }

    //Add to bottom
    m_layers.push_back(newLayer);
    return true;
}

bool KisGroupLayer::removeLayer(KisLayerSP layer)
{
    for (int layerIndex = m_layers.size() - 1; layerIndex >= 0; layerIndex--) {
        if (m_layers[layerIndex] == layer)
        {
            m_layers.erase(m_layers.begin() + layerIndex);
            layer->setParent(0);
            return true;
        }
    }

    return false;
}

KisLayerSP KisGroupLayer::firstChild() const {
    if (m_layers.count() == 0) {
        kdDebug() << "No children for firstChild!" << endl;
        return 0;
    }

    return m_layers.at(0);
}

KisLayerSP KisGroupLayer::lastChild() const {
    if (m_layers.count() == 0) {
        kdDebug() << "No children for lastChild!" << endl;
        return 0;
    }

    return m_layers.at(m_layers.count() - 1);
}

#include "kis_group_layer.moc"
