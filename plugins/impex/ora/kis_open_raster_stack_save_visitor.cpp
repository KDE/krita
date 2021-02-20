/*
 *  SPDX-FileCopyrightText: 2006-2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_open_raster_stack_save_visitor.h"

#include <math.h>

#include <QDomElement>
#include <QImage>

#include <KoCompositeOpRegistry.h>

#include "kis_adjustment_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include <generator/kis_generator_layer.h>
#include "kis_open_raster_save_context.h"
#include <kis_clone_layer.h>
#include <kis_external_layer_iface.h>

struct KisOpenRasterStackSaveVisitor::Private {
    Private() {}
    KisOpenRasterSaveContext* saveContext;
    QDomDocument layerStack;
    QDomElement currentElement;
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

    if (layer->inherits("KisGroupLayer")) {
        // Workaround for the issue regarding ora specification.
        // MyPaint treats layer's x and y relative to the group's x and y
        //  while Gimp and Krita think those are absolute values.
        // Hence we set x and y on group layers to always be 0.
        elt.setAttribute("x", QString().setNum(0));
        elt.setAttribute("y", QString().setNum(0));

    } else {
        elt.setAttribute("x", QString().setNum(layer->exactBounds().x()));
        elt.setAttribute("y", QString().setNum(layer->exactBounds().y()));
    }

    if (layer->userLocked()) {
        elt.setAttribute("edit-locked", "true");
    }
    if (d->activeNodes.contains(layer)) {
        elt.setAttribute("selected", "true");
    }
    QString compop = layer->compositeOpId();
    if (layer->compositeOpId() == COMPOSITE_CLEAR) compop = "svg:clear";
    else if (layer->compositeOpId() == COMPOSITE_ERASE) compop = "svg:dst-out";
    else if (layer->compositeOpId() == COMPOSITE_DESTINATION_ATOP) compop = "svg:dst-atop";
    else if (layer->compositeOpId() == COMPOSITE_DESTINATION_IN) compop = "svg:dst-in";
    else if (layer->compositeOpId() == COMPOSITE_ADD) compop = "svg:plus";
    else if (layer->compositeOpId() == COMPOSITE_MULT) compop = "svg:multiply";
    else if (layer->compositeOpId() == COMPOSITE_SCREEN) compop = "svg:screen";
    else if (layer->compositeOpId() == COMPOSITE_OVERLAY) compop = "svg:overlay";
    else if (layer->compositeOpId() == COMPOSITE_DARKEN) compop = "svg:darken";
    else if (layer->compositeOpId() == COMPOSITE_LIGHTEN) compop = "svg:lighten";
    else if (layer->compositeOpId() == COMPOSITE_DODGE) compop = "svg:color-dodge";
    else if (layer->compositeOpId() == COMPOSITE_BURN) compop = "svg:color-burn";
    else if (layer->compositeOpId() == COMPOSITE_HARD_LIGHT) compop = "svg:hard-light";
    else if (layer->compositeOpId() == COMPOSITE_SOFT_LIGHT_SVG) compop = "svg:soft-light";
    else if (layer->compositeOpId() == COMPOSITE_DIFF) compop = "svg:difference";
    else if (layer->compositeOpId() == COMPOSITE_COLOR) compop = "svg:color";
    else if (layer->compositeOpId() == COMPOSITE_LUMINIZE) compop = "svg:luminosity";
    else if (layer->compositeOpId() == COMPOSITE_HUE) compop = "svg:hue";
    else if (layer->compositeOpId() == COMPOSITE_SATURATION) compop = "svg:saturation";

    // it is important that the check for alphaChannelDisabled (and other non compositeOpId checks)
    // come before the check for COMPOSITE_OVER, otherwise they will be logically ignored.
    else if (layer->alphaChannelDisabled()) compop = "svg:src-atop";
    else if (layer->compositeOpId() == COMPOSITE_OVER) compop = "svg:src-over";

    //else if (layer->compositeOpId() == COMPOSITE_EXCLUSION) compop = "svg:exclusion";
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
    QDomElement previousElt = d->currentElement;

    QDomElement elt = d->layerStack.createElement("stack");
    d->currentElement = elt;
    saveLayerInfo(elt, layer);
    QString isolate = "isolate";
    if (layer->passThroughMode()) {
        isolate = "auto";
    }
    elt.setAttribute("isolation", isolate);
    visitAll(layer);

    if (!previousElt.isNull()) {
        previousElt.insertBefore(elt, QDomNode());
        d->currentElement = previousElt;
    } else {
        QDomElement imageElt = d->layerStack.createElement("image");
        int width = layer->image()->width();
        int height = layer->image()->height();
        int xRes = (int)(qRound(layer->image()->xRes() * 72));
        int yRes = (int)(qRound(layer->image()->yRes() * 72));

        imageElt.setAttribute("version", "0.0.1");
        imageElt.setAttribute("w", width);
        imageElt.setAttribute("h", height);
        imageElt.setAttribute("xres", xRes);
        imageElt.setAttribute("yres", yRes);
        imageElt.appendChild(elt);
        d->layerStack.insertBefore(imageElt, QDomNode());
        d->currentElement = QDomElement();
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
    if (layer->isFakeNode()) {
        // don't save grids, reference images layers etc.
        return true;
    }

    // here we adjust the bounds to encompass the entire area of the layer, including transforms
    QRect adjustedBounds = layer->exactBounds();

    if (adjustedBounds.isEmpty()) {
        // in case of an empty layer, artificially increase the size of the saved rectangle
        // to just save an empty layer file
        adjustedBounds.adjust(0, 0, 1, 1);
    }

    QString filename = d->saveContext->saveDeviceData(layer->projection(), layer->metaData(), adjustedBounds, layer->image()->xRes(), layer->image()->yRes());

    QDomElement elt = d->layerStack.createElement("layer");
    saveLayerInfo(elt, layer);
    elt.setAttribute("src", filename);
    d->currentElement.insertBefore(elt, QDomNode());

    return true;
}
