/*
 *  SPDX-FileCopyrightText: 2006-2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_open_raster_stack_load_visitor.h"

#include <QDomElement>
#include <QDomNode>

#include <KoColorSpaceRegistry.h>

// Includes from krita/image
#include <kis_adjustment_layer.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <KoCompositeOpRegistry.h>
#include <kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include <kis_paint_layer.h>
#include <kis_png_converter.h>
#include <kis_selection.h>
#include <kis_dom_utils.h>

#include "KisDocument.h"

#include "kis_open_raster_load_context.h"

struct KisOpenRasterStackLoadVisitor::Private {
    KisImageSP image;
    vKisNodeSP activeNodes;
    KisUndoStore* undoStore;
    KisOpenRasterLoadContext* loadContext;
    double xRes;
    double yRes;
};

KisOpenRasterStackLoadVisitor::KisOpenRasterStackLoadVisitor(KisUndoStore* undoStore, KisOpenRasterLoadContext* orlc)
        : d(new Private)
{
    d->undoStore = undoStore;
    d->loadContext = orlc;
}

KisOpenRasterStackLoadVisitor::~KisOpenRasterStackLoadVisitor()
{
    delete d;
}

KisImageSP KisOpenRasterStackLoadVisitor::image()
{
    return d->image;
}

vKisNodeSP KisOpenRasterStackLoadVisitor::activeNodes()
{
    return d->activeNodes;
}

void KisOpenRasterStackLoadVisitor::loadImage()
{

    QDomDocument doc = d->loadContext->loadStack();


    for (QDomNode node = doc.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.nodeName() == "image") { // it's the image root
            QDomElement subelem = node.toElement();

            int width = 0;
            if (!subelem.attribute("w").isNull()) {
                width = subelem.attribute("w").toInt();
            }

            int height = 0;
            if (!subelem.attribute("h").isNull()) {
                height = subelem.attribute("h").toInt();
            }

            d->xRes = 75.0/72; // Setting the default value of the X Resolution = 75ppi
            if (!subelem.attribute("xres").isNull()){
                d->xRes = (KisDomUtils::toDouble(subelem.attribute("xres")) / 72);
            }

            d->yRes = 75.0/72;
            if (!subelem.attribute("yres").isNull()){
                d->yRes = (KisDomUtils::toDouble(subelem.attribute("yres")) / 72);
            }

            dbgFile << ppVar(width) << ppVar(height);
            d->image = new KisImage(d->undoStore, width, height, KoColorSpaceRegistry::instance()->rgb8(), "OpenRaster Image (name)");
            for (QDomNode node2 = node.firstChild(); !node2.isNull(); node2 = node2.nextSibling()) {
                if (node2.isElement() && node2.nodeName() == "stack") { // it's the root layer !
                    QDomElement subelem2 = node2.toElement();
                    loadGroupLayer(subelem2, d->image->rootLayer());
                    break;
                }
            }
        }
    }
}

void KisOpenRasterStackLoadVisitor::loadLayerInfo(const QDomElement& elem, KisLayerSP layer)
{
    layer->setName(elem.attribute("name"));
    layer->setX(elem.attribute("x").toInt());
    layer->setY(elem.attribute("y").toInt());
    if (elem.attribute("visibility") == "hidden") {
        layer->setVisible(false);
    } else {
        layer->setVisible(true);
    }
    if (elem.hasAttribute("edit-locked")) {
        layer->setUserLocked(elem.attribute("edit-locked") == "true");
    }
    if (elem.hasAttribute("selected") && elem.attribute("selected") == "true") {
        d->activeNodes.append(layer);
    }

    QString compop = elem.attribute("composite-op");
    if (compop.startsWith("svg:")) {
        //we don't have a 'composite op clear' despite the registery reserving a string for it, doesn't matter, ora doesn't use it.
        //if (compop == "svg:clear") layer->setCompositeOpId(COMPOSITE_CLEAR);
        if (compop == "svg:src-over") layer->setCompositeOpId(COMPOSITE_OVER);
        //not part of the spec.
        //if (compop == "svg:dst-over") layer->setCompositeOpId(COMPOSITE_BEHIND);
        //dst-in "The source that overlaps the destination, replaces the destination."
        if (compop == "svg:dst-in") layer->setCompositeOpId(COMPOSITE_DESTINATION_IN);
        //dst-out "dst is placed, where it falls outside of the source."
        if (compop == "svg:dst-out") layer->setCompositeOpId(COMPOSITE_ERASE);
        //src-atop "Destination which overlaps the source replaces the source. Source is placed elsewhere."
        //this is basically our alpha-inherit.
        if (compop == "svg:src-atop") layer->disableAlphaChannel(true);
        //dst-atop
        if (compop == "svg:dst-atop") layer->setCompositeOpId(COMPOSITE_DESTINATION_ATOP);
        //plus is svg standard's way of saying addition... photoshop calls this linear dodge, btw, maybe make a similar alias?
        if (compop == "svg:plus") layer->setCompositeOpId(COMPOSITE_ADD);
        if (compop == "svg:multiply") layer->setCompositeOpId(COMPOSITE_MULT);
        if (compop == "svg:screen") layer->setCompositeOpId(COMPOSITE_SCREEN);
        if (compop == "svg:overlay") layer->setCompositeOpId(COMPOSITE_OVERLAY);
        if (compop == "svg:darken") layer->setCompositeOpId(COMPOSITE_DARKEN);
        if (compop == "svg:lighten") layer->setCompositeOpId(COMPOSITE_LIGHTEN);
        if (compop == "svg:color-dodge") layer->setCompositeOpId(COMPOSITE_DODGE);
        if (compop == "svg:color-burn") layer->setCompositeOpId(COMPOSITE_BURN);
        if (compop == "svg:hard-light") layer->setCompositeOpId(COMPOSITE_HARD_LIGHT);
        if (compop == "svg:soft-light") layer->setCompositeOpId(COMPOSITE_SOFT_LIGHT_SVG);
        if (compop == "svg:difference") layer->setCompositeOpId(COMPOSITE_DIFF);
        if (compop == "svg:color") layer->setCompositeOpId(COMPOSITE_COLOR);
        if (compop == "svg:luminosity") layer->setCompositeOpId(COMPOSITE_LUMINIZE);
        if (compop == "svg:hue") layer->setCompositeOpId(COMPOSITE_HUE);
        if (compop == "svg:saturation") layer->setCompositeOpId(COMPOSITE_SATURATION);
        //Exclusion isn't in the official list.
        //if (compop == "svg:exclusion") layer->setCompositeOpId(COMPOSITE_EXCLUSION);
    }
    else if (compop.startsWith("krita:")) {
        compop = compop.remove(0, 6);
        layer->setCompositeOpId(compop);
    }
    else {
        // to fix old bugs in krita's ora export
        if (compop == "color-dodge") layer->setCompositeOpId(COMPOSITE_DODGE);
        if (compop == "difference") layer->setCompositeOpId(COMPOSITE_DIFF);
        if (compop == "svg:add") layer->setCompositeOpId(COMPOSITE_ADD);
    }

}

void KisOpenRasterStackLoadVisitor::loadAdjustmentLayer(const QDomElement& elem, KisAdjustmentLayerSP aL)
{
    loadLayerInfo(elem, aL);
}

void KisOpenRasterStackLoadVisitor::loadPaintLayer(const QDomElement& elem, KisPaintLayerSP pL)
{
    loadLayerInfo(elem, pL);

    dbgFile << "Loading was unsuccessful";
}

void KisOpenRasterStackLoadVisitor::loadGroupLayer(const QDomElement& elem, KisGroupLayerSP groupLayer)
{
    dbgFile << "Loading group layer" << d->image;
    loadLayerInfo(elem, groupLayer);
    for (QDomNode node = elem.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            QDomElement subelem = node.toElement();
            if (node.nodeName() == "stack") {
                double opacity = 1.0;
                if (!subelem.attribute("opacity").isNull()) {
                    opacity = KisDomUtils::toDouble(subelem.attribute("opacity", "1.0"));
                }
                KisGroupLayerSP layer = new KisGroupLayer(d->image, "", opacity * 255);
                bool passThrough = true;
                if (subelem.attribute("isolation")=="isolate") {
                    passThrough = false;
                }
                layer->setPassThroughMode(passThrough);
                d->image->addNode(layer, groupLayer.data(), 0);
                loadGroupLayer(subelem, layer);
            } else if (node.nodeName() == "layer") {
                QString filename = subelem.attribute("src");
                if (!filename.isNull()) {
                    const qreal opacity = KisDomUtils::toDouble(subelem.attribute("opacity", "1.0"));
                    KisImageSP pngImage = d->loadContext->loadDeviceData(filename);
                    if (pngImage) {
                        // If ORA doesn't have resolution info, load the default value(75 ppi) else fetch from stack.xml
                        d->image->setResolution(d->xRes, d->yRes);
                        // now get the device
                        KisPaintDeviceSP device = pngImage->projection();

                        KisPaintLayerSP layer = new KisPaintLayer(groupLayer->image() , "", opacity * 255, device);
                        d->image->addNode(layer, groupLayer, 0);
                        loadPaintLayer(subelem, layer);
                        dbgFile << "Loading was successful";
                    }
                }
            } else if (node.nodeName() == "filter") {

                QString filterType = subelem.attribute("type");
                QStringList filterTypeSplit = filterType.split(':');
                KisFilterSP f = 0;
                if (filterTypeSplit[0] == "applications" && filterTypeSplit[1] == "krita") {
                    f = KisFilterRegistry::instance()->value(filterTypeSplit[2]);
                }
                KisFilterConfigurationSP  kfc = f->factoryConfiguration(KisGlobalResourcesInterface::instance());
                kfc->createLocalResourcesSnapshot();
                KisAdjustmentLayerSP layer = new KisAdjustmentLayer(groupLayer->image() , "", kfc, KisSelectionSP(0));
                d->image->addNode(layer.data(), groupLayer.data(), 0);
                loadAdjustmentLayer(subelem, layer);

            } else {
                dbgFile << "Unknown element : " << node.nodeName();
            }
        }
    }

}
