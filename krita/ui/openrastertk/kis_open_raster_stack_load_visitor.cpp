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

#include <kis_open_raster_stack_load_visitor.h>

#include <QDomElement>
#include <QDomNode>

#include <KoColorSpaceRegistry.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

// Includes from krita/image
#include <kis_adjustment_layer.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
#include <kis_group_layer.h>
#include <kis_image.h>

#include <kis_paint_layer.h>
#include <kis_png_converter.h>
#include <kis_selection.h>

#include "kis_doc2.h"

#include "kis_open_raster_load_context.h"

struct KisOpenRasterStackLoadVisitor::Private {
    KisImageSP image;
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

KisImageSP KisOpenRasterStackLoadVisitor::image()
{
    return d->image;
}

void KisOpenRasterStackLoadVisitor::loadImage()
{
    d->image = new KisImage(d->doc->undoAdapter(), 100, 100, KoColorSpaceRegistry::instance()->colorSpace("RGBA",""), "OpenRaster Image (name)"); // TODO: take into account width and height parameters, and metadata, when width = height = 0 use the new function from boud to get the size of the image after the layers have been loaded

    QDomDocument doc = d->loadContext->loadStack();
    
    d->image->lock();
    for (QDomNode node = doc.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.nodeName() == "stack") { // it's the root layer !
            QDomElement subelem = node.toElement();
            loadGroupLayer(subelem, d->image->rootLayer());
            return;
        }
    }
    d->image->unlock();
}

void KisOpenRasterStackLoadVisitor::loadLayerInfo(const QDomElement& elem, KisLayer* layer)
{
    layer->setName(elem.attribute("name"));
    layer->setX(elem.attribute("x").toInt());
    layer->setY(elem.attribute("y").toInt());
}

void KisOpenRasterStackLoadVisitor::loadAdjustmentLayer(const QDomElement& elem, KisAdjustmentLayerSP aL)
{
    loadLayerInfo(elem, aL.data());
}

void KisOpenRasterStackLoadVisitor::loadPaintLayer(const QDomElement& elem, KisPaintLayerSP pL)
{
    loadLayerInfo(elem, pL.data());

    dbgFile <<"Loading was unsuccessful";
}

void KisOpenRasterStackLoadVisitor::loadGroupLayer(const QDomElement& elem, KisGroupLayerSP gL)
{
    dbgFile << "Loading group layer";
    loadLayerInfo(elem, gL.data());
    for (QDomNode node = elem.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement())
        {
            QDomElement subelem = node.toElement();
            if(node.nodeName()== "stack")
            {
                quint8 opacity = 255;
                if( !subelem.attribute("opacity").isNull())
                {
                    opacity = subelem.attribute("opacity").toInt();
                }
                KisGroupLayerSP layer = new KisGroupLayer(d->image, "", opacity);
                d->image->addNode(layer.data(), gL.data(), gL->childCount() );
                loadGroupLayer(subelem, layer);
            } else if(node.nodeName()== "layer")
            {
                QString filename = subelem.attribute("src");
                if( !filename.isNull() )
                {
                    quint8 opacity = 255;
                    if( !subelem.attribute("opacity").isNull())
                    {
                        opacity = subelem.attribute("opacity").toInt();
                    }
                    KisPaintDeviceSP device = d->loadContext->loadDeviceData( filename );
                    if( device)
                    {
                        KisPaintLayerSP layer = new KisPaintLayer( gL->image() , "", opacity, device);
                        d->image->addNode(layer.data(), gL.data(), gL->childCount() );
                        loadPaintLayer(subelem, layer);
                        dbgFile <<"Loading was successful";
                    }
                }
            } else if(node.nodeName()== "filter")
            {

                QString filterType = subelem.attribute("type");
                QStringList filterTypeSplit = filterType.split(":");
                KisFilterSP f = 0;
                if(filterTypeSplit[0] == "applications" && filterTypeSplit[1] == "krita")
                {
                    f = KisFilterRegistry::instance()->value(filterTypeSplit[2]);
                }
                KisFilterConfiguration * kfc = f->defaultConfiguration(0);
                KisAdjustmentLayerSP layer = new KisAdjustmentLayer( gL->image() , "", kfc, KisSelectionSP(0));
                d->image->addNode(layer.data(), gL.data(), gL->childCount() );
                loadAdjustmentLayer(subelem, layer);

            } else {
                dbgFile << "Unknown element : " << node.nodeName();
            }
        }
    }

}
