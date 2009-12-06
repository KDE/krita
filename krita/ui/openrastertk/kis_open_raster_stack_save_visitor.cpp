/*
 *  Copyright (c) 2006-2007,2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_open_raster_stack_save_visitor.h"

#include <QDomElement>
#include <QImage>

#include <KoStore.h>

#include "kis_adjustment_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include <generator/kis_generator_layer.h>
#include "kis_open_raster_save_context.h"

struct KisOpenRasterStackSaveVisitor::Private {
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
    elt.setAttribute("opacity", layer->opacity() / 255.0);
}

bool KisOpenRasterStackSaveVisitor::visit(KisPaintLayer *layer)
{
    QString filename = d->saveContext->saveDeviceData(layer);

    QDomElement elt = d->layerStack.createElement("layer");
    saveLayerInfo(elt, layer);
    elt.setAttribute("src", filename);
    d->currentElement->insertBefore(elt, QDomNode());

    return true;
}

bool KisOpenRasterStackSaveVisitor::visit(KisGeneratorLayer* layer)
{
    Q_UNUSED(layer);
    // XXX: implement!

    return true;
}

bool KisOpenRasterStackSaveVisitor::visit(KisGroupLayer *layer)
{
    QDomElement* previousElt = d->currentElement;

    QDomElement elt = d->layerStack.createElement("stack");
    d->currentElement = &elt;
    saveLayerInfo(elt, layer);

    visitAll(layer);

    if (previousElt) {
        previousElt->insertBefore(elt, QDomNode());
        d->currentElement = previousElt;
    } else {
        QDomElement imageElt = d->layerStack.createElement("image");
        int width = layer->image()->width();
        int height = layer->image()->height();
        imageElt.setAttribute("w", width);
        imageElt.setAttribute("h", height);
        imageElt.appendChild(elt);
        d->layerStack.insertBefore(imageElt, QDomNode());
        d->currentElement = 0;
        d->saveContext->saveStack(d->layerStack);
    }

    return true;
}

bool KisOpenRasterStackSaveVisitor::visit(KisAdjustmentLayer *layer)
{
    QDomElement elt = d->layerStack.createElement("filter");
    saveLayerInfo(elt, layer);
    elt.setAttribute("type", "applications:krita:" + layer->filter()->name());
    saveLayerInfo(elt, layer);
    d->currentElement->insertBefore(elt, QDomNode());
    return true;
}
