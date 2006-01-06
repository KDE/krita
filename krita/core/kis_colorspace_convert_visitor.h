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
#include "kis_paint_device_impl.h"

class KisColorSpaceConvertVisitor :public KisLayerVisitor {
public:
    KisColorSpaceConvertVisitor(KisColorSpace *dstColorSpace, Q_INT32 renderingIntent);
    virtual ~KisColorSpaceConvertVisitor();

public:
    virtual bool visit(KisPaintLayer *layer);
    virtual bool visit(KisGroupLayer *layer);
    virtual bool visit(KisPartLayer *layer);

private:
    KisColorSpace *m_dstColorSpace;
    Q_INT32 m_renderingIntent;
};

KisColorSpaceConvertVisitor::KisColorSpaceConvertVisitor(KisColorSpace *dstColorSpace, Q_INT32 renderingIntent) :
    KisLayerVisitor(),
    m_dstColorSpace(dstColorSpace),
    m_renderingIntent(renderingIntent)
{
}

KisColorSpaceConvertVisitor::~KisColorSpaceConvertVisitor()
{
}

bool KisColorSpaceConvertVisitor::visit(KisGroupLayer * layer)
{
    KisLayerSP child = layer->firstChild();
    while (child) {
        child->accept(*this);
        child = child->nextSibling();
    }

    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisPaintLayer *layer)
{
    layer -> paintDevice() -> convertTo(m_dstColorSpace, m_renderingIntent);
    return true;
}

bool KisColorSpaceConvertVisitor::visit(KisPartLayer *)
{
    return true;
}

#endif // KIS_COLORSPACE_CONVERT_VISITOR_H_

