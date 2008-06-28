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

#include "kra/kis_savexml_visitor.h"

#include "kis_adjustment_layer.h"
#include "filter/kis_filter_configuration.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_shape_layer.h"
#include "generator/kis_generator_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_transformation_mask.h"
#include "kis_selection_mask.h"
#include "kis_clone_layer.h"
#include <KoCompositeOp.h>
#include <kis_paint_device.h>
#include <KoColorSpace.h>

KisSaveXmlVisitor::KisSaveXmlVisitor(QDomDocument doc, const QDomElement & element, quint32 &count, bool root) :
    KisNodeVisitor(),
    m_doc(doc),
    m_count(count),
    m_root(root)
{
    dbgKrita << " creating savexml visitor ";
    m_elem = element;
}

bool KisSaveXmlVisitor::visit( KisExternalLayer * layer )
{
    if (KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(layer)) {
        QDomElement layerElement = m_doc.createElement("layer");
        saveLayer(layerElement, "shapelayer", layer);
        layerElement.setAttribute("x", shapeLayer->x());
        layerElement.setAttribute("y", shapeLayer->y());
        m_elem.appendChild(layerElement);
        m_count++;
        return saveMasks( layer, layerElement );
    }
    return false;
}

bool KisSaveXmlVisitor::visit(KisPaintLayer *layer)
{
    QDomElement layerElement = m_doc.createElement("layer");
    saveLayer(layerElement, "paintlayer", layer);
    layerElement.setAttribute("x", layer->x());
    layerElement.setAttribute("y", layer->y());
    layerElement.setAttribute("colorspacename", layer->paintDevice()->colorSpace()->id());
    m_elem.appendChild(layerElement);

/*    if(layer->paintDevice()->hasExifInfo())
    {
        QDomElement exifElmt = layer->paintDevice()->exifInfo()->save(m_doc);
        layerElement.appendChild(exifElmt);
    } TODO: save the metadata
*/
    m_count++;
    return saveMasks( layer, layerElement );
}

bool KisSaveXmlVisitor::visit(KisGroupLayer *layer)
{
    QDomElement layerElement;

    if(m_root) // if this is the root we fake so not to save it
        layerElement = m_elem;
    else
    {
        QDomElement layerElement = m_doc.createElement("layer");
        saveLayer(layerElement, "grouplayer", layer);

        layerElement.setAttribute("x", layer->x());
        layerElement.setAttribute("y", layer->y());
        m_elem.appendChild(layerElement);
    }

    QDomElement elem = m_doc.createElement("LAYERS");
    layerElement.appendChild(elem);
    KisSaveXmlVisitor visitor(m_doc, elem, m_count);

    return visitor.visitAllInverse( layer );
}

bool KisSaveXmlVisitor::visit(KisAdjustmentLayer* layer)
{
    QDomElement layerElement = m_doc.createElement("layer");
    saveLayer(layerElement, "adjustmentlayer", layer);
    layerElement.setAttribute("filtername", layer->filter()->name());
    layerElement.setAttribute("filterversion", layer->filter()->version());
    layerElement.setAttribute("x", layer->x());
    layerElement.setAttribute("y", layer->y());
    m_elem.appendChild(layerElement);

    m_count++;
    return saveMasks( layer, layerElement );
}

bool KisSaveXmlVisitor::visit(KisGeneratorLayer *layer)
{
    QDomElement layerElement = m_doc.createElement("layer");
    saveLayer(layerElement, "generatorlayer", layer);
    layerElement.setAttribute("generatorname", layer->generator()->name());
    layerElement.setAttribute("generatorversion", layer->generator()->version());
    layerElement.setAttribute("x", layer->x());
    layerElement.setAttribute("y", layer->y());
    m_elem.appendChild(layerElement);

    m_count++;
    return saveMasks( layer, layerElement );
}

bool KisSaveXmlVisitor::visit(KisCloneLayer *layer)
{
    QDomElement layerElement = m_doc.createElement("layer");
    saveLayer(layerElement, "clonelayer", layer);
    layerElement.setAttribute("x", layer->x());
    layerElement.setAttribute("y", layer->y());
    layerElement.setAttribute("clonefrom", layer->copyFrom()->name());
    layerElement.setAttribute("clonetype", layer->copyType());
    m_elem.appendChild(layerElement);

    m_count++;
    return saveMasks( layer, layerElement );
}

bool KisSaveXmlVisitor::visit(KisFilterMask *mask)
{
    dbgKrita << "filtermask";
    QDomElement el = m_doc.createElement("mask");
    saveMask(el, "filtermask", mask);
    el.setAttribute("filtername", mask->filter()->name());
    el.setAttribute("filterversion", mask->filter()->version());

    m_elem.appendChild(el);

    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisTransparencyMask *mask)
{
    dbgKrita << "transparency mask";
    QDomElement el = m_doc.createElement("mask");
    saveMask(el, "transparencymask", mask);
    m_elem.appendChild(el);
    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisTransformationMask *mask)
{
    dbgKrita << "transformationmask";

    QDomElement el = m_doc.createElement("mask");
    saveMask(el, "transformationmask", mask);
    el.setAttribute("x_scale", mask->xScale());
    el.setAttribute("y_scale", mask->yScale());
    el.setAttribute("x_shear", mask->xShear());
    el.setAttribute("y_shear", mask->yShear());
    el.setAttribute("rotation", mask->rotation());
    el.setAttribute("x_translation", mask->xTranslate());
    el.setAttribute("y_translation", mask->yTranslate());
    el.setAttribute("fiter_strategy", mask->filterStrategy()->name());
    m_elem.appendChild(el);
    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisSelectionMask *mask)
{
    dbgKrita << "selectionmask";

    QDomElement el = m_doc.createElement("mask");
    saveMask(el, "selectionmask", mask);
    m_elem.appendChild(el);
    m_count++;
    return true;
}


void KisSaveXmlVisitor::saveLayer(QDomElement & el, const QString & layerType, const KisLayer * layer)
{

    el.setAttribute("name", layer->KisBaseNode::name());
    el.setAttribute("opacity", layer->opacity());
    el.setAttribute("compositeop", layer->compositeOp()->id());
    el.setAttribute("visible", layer->visible());
    el.setAttribute("locked", layer->locked());
    el.setAttribute("layertype", layerType);
    el.setAttribute("filename", QString("layer%1").arg(m_count));

}

void KisSaveXmlVisitor::saveMask(QDomElement & el, const QString & maskType, const KisMask * mask)
{
    el.setAttribute("name", mask->KisBaseNode::name());
    el.setAttribute("visible", mask->visible());
    el.setAttribute("locked", mask->locked());
    el.setAttribute("masktype", maskType);
    el.setAttribute("filename", QString("mask%1").arg(m_count));
    el.setAttribute("x", mask->x());
    el.setAttribute("y", mask->y());
}

bool KisSaveXmlVisitor::saveMasks(KisNode * node, QDomElement & layerElement)
{
    if (node->childCount() > 0) {
        QDomElement elem = m_doc.createElement("MASKS");
        layerElement.appendChild(elem);
        KisSaveXmlVisitor visitor(m_doc, elem, m_count);
        return visitor.visitAllInverse( node );
    }
    return true;
}


