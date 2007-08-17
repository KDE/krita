/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <kis_savexml_visitor.h>

#include "kis_adjustment_layer.h"
#include "kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"

KisSaveXmlVisitor::KisSaveXmlVisitor(QDomDocument doc, QDomElement element, quint32 &count, bool root) :
    KisNodeVisitor(),
    m_doc(doc),
    m_count(count),
    m_root(root)
{
    m_elem = element;
}

bool KisSaveXmlVisitor::visit( KisExternalLayer * )
{
    return true;
}

bool KisSaveXmlVisitor::visit(KisPaintLayer *layer)
{
    QDomElement layerElement = m_doc.createElement("layer");

    layerElement.setAttribute("name", layer->name());
    layerElement.setAttribute("x", layer->x());
    layerElement.setAttribute("y", layer->y());
    layerElement.setAttribute("opacity", layer->opacity());
    layerElement.setAttribute("compositeop", layer->compositeOp()->id());
    layerElement.setAttribute("visible", layer->visible());
    layerElement.setAttribute("locked", layer->locked());
    layerElement.setAttribute("layertype", "paintlayer");
    layerElement.setAttribute("filename", QString("layer%1").arg(m_count));
    layerElement.setAttribute("colorspacename", layer->paintDevice()->colorSpace()->id());
//     layerElement.setAttribute("hasmask", layer->hasMask());

    m_elem.appendChild(layerElement);

/*    if(layer->paintDevice()->hasExifInfo())
    {
        QDomElement exifElmt = layer->paintDevice()->exifInfo()->save(m_doc);
        layerElement.appendChild(exifElmt);
    } TODO: save the metadata*/
    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisGroupLayer *layer)
{
    QDomElement layerElement;

    if(m_root) // if this is the root we fake so not to save it
        layerElement = m_elem;
    else
    {
        layerElement = m_doc.createElement("layer");

        layerElement.setAttribute("name", layer->name());
        layerElement.setAttribute("x", layer->x());
        layerElement.setAttribute("y", layer->y());
        layerElement.setAttribute("opacity", layer->opacity());
        layerElement.setAttribute("compositeop", layer->compositeOp()->id());
        layerElement.setAttribute("visible", layer->visible());
        layerElement.setAttribute("locked", layer->locked());
        layerElement.setAttribute("layertype", "grouplayer");

        m_elem.appendChild(layerElement);
    }

    QDomElement elem = m_doc.createElement("LAYERS");

    layerElement.appendChild(elem);

    KisSaveXmlVisitor visitor(m_doc, elem, m_count);

    return visitAll( layer );
}

bool KisSaveXmlVisitor::visit(KisAdjustmentLayer* layer)
{
    QDomElement layerElement = m_doc.createElement("layer");

    layerElement.setAttribute("name", layer->name());
    layerElement.setAttribute("filtername", layer->filter()->name());
    layerElement.setAttribute("filterversion", layer->filter()->version());
    layerElement.setAttribute("opacity", layer->opacity());
    layerElement.setAttribute("compositeop", layer->compositeOp()->id());
    layerElement.setAttribute("visible", layer->visible());
    layerElement.setAttribute("locked", layer->locked());
    layerElement.setAttribute("layertype", "adjustmentlayer");
    layerElement.setAttribute("filename", QString("layer%1").arg(m_count));
    layerElement.setAttribute("x", layer->x());
    layerElement.setAttribute("y", layer->y());
    m_elem.appendChild(layerElement);

    m_count++;
    return true;
}

