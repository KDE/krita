/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_COLORSPACE_CONVERT_VISITOR_H_
#define KIS_COLORSPACE_CONVERT_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer_visitor.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"

class KoColorSpaceConvertVisitor :public KisLayerVisitor {
public:
    KoColorSpaceConvertVisitor(KoColorSpace *dstColorSpace, qint32 renderingIntent);
    virtual ~KoColorSpaceConvertVisitor();

public:
    virtual bool visit(KisPaintLayer *layer);
    virtual bool visit(KisGroupLayer *layer);
    virtual bool visit(KisPartLayer *layer);
    virtual bool visit(KisAdjustmentLayer* layer);
    
private:
    KoColorSpace *m_dstColorSpace;
    qint32 m_renderingIntent;
};

KoColorSpaceConvertVisitor::KoColorSpaceConvertVisitor(KoColorSpace *dstColorSpace, qint32 renderingIntent) :
    KisLayerVisitor(),
    m_dstColorSpace(dstColorSpace),
    m_renderingIntent(renderingIntent)
{
}

KoColorSpaceConvertVisitor::~KoColorSpaceConvertVisitor()
{
}

bool KoColorSpaceConvertVisitor::visit(KisGroupLayer * layer)
{
    // Clear the projection, we will have to re-render everything.
    // The image is already set to the new colorspace, so this'll work.
    layer->resetProjection();
    
    KisLayerSP child = layer->firstChild();
    while (child) {
        child->accept(*this);
        child = child->nextSibling();
    }
    layer->setDirty();
    return true;
}

bool KoColorSpaceConvertVisitor::visit(KisPaintLayer *layer)
{
    layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent);

    layer->setDirty();
    return true;
}

bool KoColorSpaceConvertVisitor::visit(KisPartLayer *)
{
    return true;
}


bool KoColorSpaceConvertVisitor::visit(KisAdjustmentLayer * layer)
{
    if (layer->filter()->name() == "perchannel") {
        // Per-channel filters need to be reset because of different number
        // of channels. This makes undo very tricky, but so be it.
        // XXX: Make this more generic for after 1.6, when we'll have many
        // channel-specific filters. 
        KisFilterSP f = KisFilterRegistry::instance()->get("perchannel");
        layer->setFilter(f->configuration());
    }
    layer->resetCache();
    layer->setDirty();
    return true;
}

#endif // KIS_COLORSPACE_CONVERT_VISITOR_H_

