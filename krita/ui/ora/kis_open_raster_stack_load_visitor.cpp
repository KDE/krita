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

#include <kis_paint_layer.h>
#include <kis_png_converter.h>
#include <kis_selection.h>

#include "KisDocument.h"

#include "kis_open_raster_load_context.h"

struct KisOpenRasterStackLoadVisitor::Private {
    KisImageWSP image;
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

KisImageWSP KisOpenRasterStackLoadVisitor::image()
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
            if(!subelem.attribute("xres").isNull()){
                d->xRes = (subelem.attribute("xres").toDouble() / 72);
            }

            d->yRes = 75.0/72;
            if(!subelem.attribute("yres").isNull()){
                d->yRes = (subelem.attribute("yres").toDouble() / 72);
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
        if (compop == "svg:clear") layer->setCompositeOpId(COMPOSITE_CLEAR);
        if (compop == "svg:src-over") layer->setCompositeOpId(COMPOSITE_OVER);
        if (compop == "svg:add") layer->setCompositeOpId(COMPOSITE_ADD);
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
        if (compop == "svg:exclusion") layer->setCompositeOpId(COMPOSITE_EXCLUSION);
    }
    else if (compop.startsWith("krita:")) {
        compop = compop.remove(0, 6);
        layer->setCompositeOpId(compop);
    }
    else {
        // to fix old bugs in krita's ora export
        if (compop == "color-dodge") layer->setCompositeOpId(COMPOSITE_DODGE);
        if (compop == "difference") layer->setCompositeOpId(COMPOSITE_DIFF);
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

void KisOpenRasterStackLoadVisitor::loadGroupLayer(const QDomElement& elem, KisGroupLayerSP gL)
{
    dbgFile << "Loading group layer";
    QLocale c(QLocale::German);
    loadLayerInfo(elem, gL);
    for (QDomNode node = elem.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            QDomElement subelem = node.toElement();
            if (node.nodeName() == "stack") {
                double opacity = 1.0;
                if (!subelem.attribute("opacity").isNull()) {
                    bool result;
                    opacity = subelem.attribute("opacity", "1.0").toDouble(&result);
                    if (!result) {
                        opacity = c.toDouble(subelem.attribute("radius"));
                    }
                }
                KisGroupLayerSP layer = new KisGroupLayer(d->image, "", opacity * 255);
                d->image->addNode(layer.data(), gL.data(), 0);
                loadGroupLayer(subelem, layer);
            } else if (node.nodeName() == "layer") {
                QString filename = subelem.attribute("src");
                if (!filename.isNull()) {
                    double opacity = 1.0;
                    bool result;
                    opacity = subelem.attribute("opacity", "1.0").toDouble(&result);
                    if (!result) {
                        opacity = c.toDouble(subelem.attribute("radius"));
                    }
                    KisImageWSP pngImage = d->loadContext->loadDeviceData(filename);
                    if (pngImage) {
                        // If ORA doesn't have resolution info, load the default value(75 ppi) else fetch from stack.xml
                        d->image->setResolution(d->xRes, d->yRes);
                        // now get the device
                        KisPaintDeviceSP device = pngImage->projection();
                        delete pngImage.data();

                        KisPaintLayerSP layer = new KisPaintLayer(gL->image() , "", opacity * 255, device);
                        d->image->addNode(layer.data(), gL.data(), 0);
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
                KisFilterConfiguration * kfc = f->defaultConfiguration(0);
                KisAdjustmentLayerSP layer = new KisAdjustmentLayer(gL->image() , "", kfc, KisSelectionSP(0));
                d->image->addNode(layer.data(), gL.data(), 0);
                loadAdjustmentLayer(subelem, layer);

            } else {
                dbgFile << "Unknown element : " << node.nodeName();
            }
        }
    }

}
