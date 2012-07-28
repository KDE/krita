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
#include <KoCompositeOp.h>

#include "kis_adjustment_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include <generator/kis_generator_layer.h>
#include "kis_open_raster_save_context.h"
#include <kis_clone_layer.h>
#include <generator/kis_generator_layer.h>
#include <kis_external_layer_iface.h>

struct KisOpenRasterStackSaveVisitor::Private {
    Private() : currentElement(0) {}
    KisOpenRasterSaveContext* saveContext;
    QDomDocument layerStack;
    QDomElement* currentElement;
    vKisNodeSP activeNodes;
};

KisOpenRasterStackSaveVisitor::KisOpenRasterStackSaveVisitor(KisOpenRasterSaveContext* saveContext, vKisNodeSP activeNodes)
    : d(new Private)
{
    d->saveContext = saveContext;
    d->activeNodes = activeNodes;
}

KisOpenRasterStackSaveVisitor::~KisOpenRasterStackSaveVisitor()
{
    delete d;
}

void KisOpenRasterStackSaveVisitor::saveLayerInfo(QDomElement& elt, KisLayer* layer)
{
    elt.setAttribute("name", layer->name());
    elt.setAttribute("opacity", QString().setNum(layer->opacity() / 255.0));
    elt.setAttribute("visibility", layer->visible() ? "visible" : "hidden");
    if (layer->userLocked()) {
        elt.setAttribute("edit-locked", "true");
    }
    if (d->activeNodes.contains(layer)) {
        elt.setAttribute("selected", "true");
    }
    QString compop = layer->compositeOpId();
    if (layer->compositeOpId() == COMPOSITE_CLEAR) compop = "svg:clear";
    else if (layer->compositeOpId() == COMPOSITE_OVER) compop = "svg:src-over";
    else if (layer->compositeOpId() == COMPOSITE_ADD) compop = "svg:add";
    else if (layer->compositeOpId() == COMPOSITE_MULT) compop = "svg:multiply";
    else if (layer->compositeOpId() == COMPOSITE_SCREEN) compop = "svg:screen";
    else if (layer->compositeOpId() == COMPOSITE_OVERLAY) compop = "svg:overlay";
    else if (layer->compositeOpId() == COMPOSITE_DARKEN) compop = "svg:darken";
    else if (layer->compositeOpId() == COMPOSITE_LIGHTEN) compop = "svg:lighten";
    else if (layer->compositeOpId() == COMPOSITE_DODGE) compop = "color-dodge";
    else if (layer->compositeOpId() == COMPOSITE_BURN) compop = "svg:color-burn";
    else if (layer->compositeOpId() == COMPOSITE_HARD_LIGHT) compop = "svg:hard-light";
    else if (layer->compositeOpId() == COMPOSITE_SOFT_LIGHT) compop = "svg:soft-light";
    else if (layer->compositeOpId() == COMPOSITE_DIFF) compop = "svg:difference";
    else if (layer->compositeOpId() == COMPOSITE_COLOR) compop = "svg:color";
    else if (layer->compositeOpId() == COMPOSITE_LUMINIZE) compop = "svg:luminosity";
    else if (layer->compositeOpId() == COMPOSITE_HUE) compop = "svg:hue";
    else if (layer->compositeOpId() == COMPOSITE_SATURATION) compop = "svg:saturation";
    else compop = "krita:" + layer->compositeOpId();
    elt.setAttribute("composite-op", compop);
}

bool KisOpenRasterStackSaveVisitor::visit(KisPaintLayer *layer)
{
    return saveLayer(layer);
}

bool KisOpenRasterStackSaveVisitor::visit(KisGeneratorLayer* layer)
{
    return saveLayer(layer);
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
    return true;
}

bool KisOpenRasterStackSaveVisitor::visit(KisCloneLayer *layer)
{
    return saveLayer(layer);
}

bool KisOpenRasterStackSaveVisitor::visit(KisExternalLayer * layer)
{
    return saveLayer(layer);
}

bool KisOpenRasterStackSaveVisitor::saveLayer(KisLayer *layer)
{
    QString filename = d->saveContext->saveDeviceData(layer->projection(), layer->metaData(), layer->image());

    QDomElement elt = d->layerStack.createElement("layer");
    saveLayerInfo(elt, layer);
    elt.setAttribute("src", filename);
    d->currentElement->insertBefore(elt, QDomNode());

    return true;
}
