/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include <kis_open_raster_stack_save_visitor.h>

#include <QDomElement>
#include <QImage>

#include <KoStore.h>

#include "kis_adjustment_layer.h"
#include "kis_filter.h"
#include "kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"

#include "kis_open_raster_save_context.h"

struct KisOpenRasterStackSaveVisitor::Private
{
    Private() : currentElement(0) {}
    KisOpenRasterSaveContext* saveContext;
    QDomDocument layerStack;
    QDomElement* currentElement;
};

KisOpenRasterStackSaveVisitor::KisOpenRasterStackSaveVisitor(KisOpenRasterSaveContext* saveContext) : d(new Private)
{
    d->saveContext = saveContext;
}

KisOpenRasterStackSaveVisitor::~KisOpenRasterStackSaveVisitor()
{
    delete d;
}

void KisOpenRasterStackSaveVisitor::saveLayerInfo(QDomElement& elt, KisLayer* layer)
{
    elt.setAttribute("name", layer->name());
    elt.setAttribute("x", layer->x());
    elt.setAttribute("y", layer->y());
    elt.setAttribute("opacity", layer->opacity());
}

bool KisOpenRasterStackSaveVisitor::visit(KisPaintLayer *layer)
{
    QString filename = d->saveContext->saveDeviceData( layer );
    
    QDomElement elt = d->layerStack.createElement("layer");
    saveLayerInfo(elt, layer);
    elt.setAttribute("src", filename);
    d->currentElement->appendChild( elt );
    
    return true;
}

bool KisOpenRasterStackSaveVisitor::visit(KisGroupLayer *layer)
{
    QDomElement* previousElt = d->currentElement;
    
    QDomElement elt = d->layerStack.createElement("stack");
    d->currentElement = &elt;
    saveLayerInfo(elt, layer );

    visitAll( layer );

    if( previousElt)
    {
        previousElt->appendChild(elt);
        d->currentElement = previousElt;
    } else {
        d->layerStack.appendChild( elt );
        d->currentElement = 0;
        d->saveContext->saveStack( d->layerStack );
    }
    
    return true;
}

bool KisOpenRasterStackSaveVisitor::visit(KisAdjustmentLayer *layer)
{
    QDomElement elt = d->layerStack.createElement("filter");
    saveLayerInfo(elt, layer);
    elt.setAttribute("type", "applications:krita:" + layer->filter()->name() );
    saveLayerInfo(elt, layer);
    d->currentElement->appendChild( elt );
    return true;
}
