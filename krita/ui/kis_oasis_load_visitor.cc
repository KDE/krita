/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include <kis_oasis_load_visitor.h>

#include <QDomElement>
#include <QDomNode>

#include <KoColorSpaceRegistry.h>
#include <KoOasisStore.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

// Includes from krita/image
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_meta_registry.h>
#include <kis_paint_layer.h>

#include "kis_doc2.h"

void KisOasisLoadVisitor::loadImage(const QDomElement& elem)
{
    m_image = new KisImage(m_doc->undoAdapter(), 100, 100, KoColorSpaceRegistry::instance()->colorSpace("RGBA",""), "OpenRaster Image (name)"); // TODO: take into account width and height parameters, and metadata, when width = height = 0 use the new function from boud to get the size of the image after the layers have been loaded

    m_image->blockSignals( true );  // Don't send out signals while we're building the image
    for (QDomNode node = elem.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.nodeName() == "image:stack") { // it's the root layer !
            QDomElement subelem = node.toElement();
            loadGroupLayer(subelem, m_image->rootLayer());
            return;
        }
    }
    m_image->blockSignals( false );
    m_image = KisImageSP(0);
}

void KisOasisLoadVisitor::loadLayerInfo(const QDomElement& elem, KisLayer* layer)
{
    layer->setName(elem.attribute("name"));
    layer->setX(elem.attribute("x").toInt());
    layer->setY(elem.attribute("y").toInt());
}


void KisOasisLoadVisitor::loadPaintLayer(const QDomElement& elem, KisPaintLayerSP pL)
{
    loadLayerInfo(elem, pL.data());
    
    QString filename = m_layerFilenames[pL.data()];
    kDebug(41008) << "Loading file : " << filename << endl;
    if (m_oasisStore->store()->open(filename) ) {
        KoStoreDevice io ( m_oasisStore->store() );
        if ( !io.open( QIODevice::ReadOnly ) )
        {
            kDebug(41008) << "Couldn't open for reading: " << filename << endl;
//             return false;
        }
        QImage img;
        if ( ! img.load( &io, "PNG" ) )
        {
            kDebug(41008) << "Loading PNG failed: " << filename << endl;
            m_oasisStore->store()->close();
            io.close();
//             return false;
        }
        pL->paintDevice()->convertFromQImage(img, "");
        io.close();
        m_oasisStore->store()->close();
        kDebug(41008) << "Loading was successful" << endl;
//         return true;
    }
    kDebug(41008) << "Loading was unsuccessful" << endl;
}

void KisOasisLoadVisitor::loadGroupLayer(const QDomElement& elem, KisGroupLayerSP gL)
{
    loadLayerInfo(elem, gL.data());
    for (QDomNode node = elem.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement())
        {
            QDomElement subelem = node.toElement();
            if(node.nodeName()== "image:stack")
            {
                quint8 opacity = 255;
                if( not subelem.attribute("opacity").isNull())
                {
                    opacity = subelem.attribute("opacity").toInt();
                }
                KisGroupLayerSP layer = new KisGroupLayer(m_image.data(), "", opacity);
                gL->addLayer(layer, gL->childCount() );
                loadGroupLayer(subelem, layer);
            } else if(node.nodeName()== "image:layer")
            {
                QString srcAttr = subelem.attribute("src");
                if( not srcAttr.isNull() )
                {
                    quint8 opacity = 255;
                    if( not subelem.attribute("opacity").isNull())
                    {
                        opacity = subelem.attribute("opacity").toInt();
                    }
                    KisPaintLayerSP layer = new KisPaintLayer( m_image.data(), "", opacity); // TODO: support of colorspacess
                    m_layerFilenames[layer.data()] = srcAttr;
                    gL->addLayer(layer, gL->childCount() );
                    loadPaintLayer(subelem, layer);
                }
            }
        }
    }

}
