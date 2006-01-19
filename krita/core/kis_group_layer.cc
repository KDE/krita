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
#include <kglobal.h>
#include <qimage.h>

#include "kis_layer.h"
#include "kis_layer_visitor.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_device_impl.h"
#include "kis_group_layer.h"

KisGroupLayer::KisGroupLayer(KisImage *img, const QString &name, Q_UINT8 opacity) :
    super(img, name, opacity)
{
    m_projection = new KisPaintDeviceImpl(img, img->colorSpace());
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) : super(rhs)
{
    for(vKisLayerSP_cit it = rhs.m_layers.begin(); it != rhs.m_layers.end(); ++it)
    {
        m_layers.push_back( it->data()->clone() );
    }
    m_projection = rhs.m_projection;
}

KisLayerSP KisGroupLayer::clone() const
{
    return new KisGroupLayer(*this);
}

KisGroupLayer::~KisGroupLayer()
{
}

uint KisGroupLayer::childCount() const
{
    return m_layers.count();
}

KisLayerSP KisGroupLayer::firstChild() const
{
    return at(0);
}

KisLayerSP KisGroupLayer::lastChild() const
{
    return at(childCount() - 1);
}

KisLayerSP KisGroupLayer::at(int index) const
{
    if (childCount() && index >= 0 && kClamp(uint(index), uint(0), childCount() - 1) == uint(index))
        return m_layers.at(reverseIndex(index));
    return 0;
}

int KisGroupLayer::index(KisLayerSP layer) const
{
    if (layer -> parent().data() == this)
        return layer -> index();
    return -1;
}

void KisGroupLayer::setIndex(KisLayerSP layer, int index)
{
    if (layer -> parent().data() != this)
        return;
    //TODO optimize
    removeLayer(layer);
    addLayer(layer, index);
}

bool KisGroupLayer::addLayer(KisLayerSP newLayer, int x)
{
    if (x < 0 || kClamp(uint(x), uint(0), childCount()) != uint(x) ||
        newLayer -> parent() || m_layers.contains(newLayer))
    {
        kdWarning() << "invalid input to KisGroupLayer::addLayer(KisLayerSP newLayer, int x)!" << endl;
        //kdDebug() << "layer: " << newLayer << ", x: " << x
        //        << ", parent: " << newLayer->parent() << ", contains: " << m_layers.contains(newLayer)
        //          << " stack: " << kdBacktrace() << "\n";
        return false;
    }
    uint index(x);
    if (index == 0)
        m_layers.append(newLayer);
    else
        m_layers.insert(m_layers.begin() + reverseIndex(index) + 1, newLayer);
    for (uint i = childCount() - 1; i > index; i--)
        at(i) -> m_index++;
    newLayer -> m_parent = this;
    newLayer -> m_index = index;
    return true;
}

bool KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)
{
    if (aboveThis && aboveThis -> parent().data() != this)
    {
        kdWarning() << "invalid input to KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)!" << endl;
        return false;
    }

    return addLayer(newLayer, aboveThis ? aboveThis -> index() : childCount());
}

bool KisGroupLayer::removeLayer(int x)
{
    if (x >= 0 && kClamp(uint(x), uint(0), childCount() - 1) == uint(x))
    {
        uint index(x);
        for (uint i = childCount() - 1; i > index; i--)
            at(i) -> m_index--;
        at(index) -> m_parent = 0;
        at(index) -> m_index = -1;
        m_layers.erase(m_layers.begin() + reverseIndex(index));
        return true;
    }
    kdWarning() << "invalid input to KisGroupLayer::removeLayer()!" << endl;
    return false;
}

bool KisGroupLayer::removeLayer(KisLayerSP layer)
{
    if (layer -> parent().data() != this)
    {
        kdWarning() << "invalid input to KisGroupLayer::removeLayer()!" << endl;
        return false;
    }
    return removeLayer(layer -> index());
}


#include "kis_group_layer.moc"
