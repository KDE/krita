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
#include <KoStore.h>
#include <KoStoreDevice.h>

// Includes from krita/image
#include <kis_adjustment_layer.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <KoCompositeOp.h>

#include <kis_paint_layer.h>
#include <kis_png_converter.h>
#include <kis_selection.h>

#include "kis_doc2.h"

#include "kis_open_raster_load_context.h"

struct KisOpenRasterStackLoadVisitor::Private {
    KisImageWSP image;
    vKisNodeSP activeNodes;
    KisDoc2* doc;
    KisOpenRasterLoadContext* loadContext;
};

KisOpenRasterStackLoadVisitor::KisOpenRasterStackLoadVisitor(KisDoc2* doc, KisOpenRasterLoadContext* orlc)
        : d(new Private)
{
    d->doc = doc;
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

            dbgFile << ppVar(width) << ppVar(height);

            d->image = new KisImage(d->doc->createUndoStore(), width, height, KoColorSpaceRegistry::instance()->rgb8(), "OpenRaster Image (name)");

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
        if (compop == "svg:clear") layer->setCompositeOp(COMPOSITE_CLEAR);
        if (compop == "svg:src-over") layer->setCompositeOp(COMPOSITE_OVER);
        if (compop == "svg:add") layer->setCompositeOp(COMPOSITE_ADD);
        if (compop == "svg:multiply") layer->setCompositeOp(COMPOSITE_MULT);
        if (compop == "svg:screen") layer->setCompositeOp(COMPOSITE_SCREEN);
        if (compop == "svg:overlay") layer->setCompositeOp(COMPOSITE_OVERLAY);
        if (compop == "svg:darken") layer->setCompositeOp(COMPOSITE_DARKEN);
        if (compop == "svg:lighten") layer->setCompositeOp(COMPOSITE_LIGHTEN);
        if (compop == "svg:color-dodge") layer->setCompositeOp(COMPOSITE_DODGE);
        if (compop == "svg:color-burn") layer->setCompositeOp(COMPOSITE_BURN);
        if (compop == "svg:hard-light") layer->setCompositeOp(COMPOSITE_HARD_LIGHT);
        if (compop == "svg:soft-light") layer->setCompositeOp(COMPOSITE_SOFT_LIGHT);
        if (compop == "svg:difference") layer->setCompositeOp(COMPOSITE_DIFF);
        if (compop == "difference") layer->setCompositeOp(COMPOSITE_DIFF); // to fix an old bug in krita's ora export
        if (compop == "svg:color") layer->setCompositeOp(COMPOSITE_COLOR);
        if (compop == "svg:luminosity") layer->setCompositeOp(COMPOSITE_LUMINIZE);
        if (compop == "svg:hue") layer->setCompositeOp(COMPOSITE_HUE);
        if (compop == "svg:saturation") layer->setCompositeOp(COMPOSITE_SATURATION);

    }
    else if (compop.startsWith("krita:")) {
        compop = compop.remove(0, 6);
        layer->setCompositeOp(compop);
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
                    KisPaintDeviceSP device = d->loadContext->loadDeviceData(filename);
                    if (device) {
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
