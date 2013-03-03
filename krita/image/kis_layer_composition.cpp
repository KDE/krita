/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_layer_composition.h"
#include "kis_node_visitor.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_paint_layer.h"
#include "generator/kis_generator_layer.h"
#include "kis_clone_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"

#include <QDomDocument>

class KisCompositionVisitor : public KisNodeVisitor {
public:
    enum Mode {
        STORE,
        APPLY
    };
    
    KisCompositionVisitor(KisLayerComposition* layerComposition, Mode mode) : m_layerComposition(layerComposition), m_mode(mode)
    {        
    }

    virtual bool visit(KisNode* node) { return process(node); }
    virtual bool visit(KisGroupLayer* layer)
    { 
        bool result = visitAll(layer);
        if(layer == layer->image()->rootLayer()) {
            return result;
        }        
        return result && process(layer);
    }
    virtual bool visit(KisAdjustmentLayer* layer) { return process(layer); }
    virtual bool visit(KisPaintLayer* layer) { return process(layer); }
    virtual bool visit(KisExternalLayer* layer) { return process(layer); }
    virtual bool visit(KisGeneratorLayer* layer) { return process(layer); }
    virtual bool visit(KisCloneLayer* layer) { return process(layer); }
    virtual bool visit(KisFilterMask* mask) { return process(mask); }
    virtual bool visit(KisTransparencyMask* mask) { return process(mask); }
    virtual bool visit(KisSelectionMask* mask) { return process(mask); }

    bool process(KisNode* node) {
        if(m_mode == STORE) {
            m_layerComposition->m_visibilityMap[node->uuid()] = node->visible();
        } else {
            bool newState = false;
            if(m_layerComposition->m_visibilityMap.contains(node->uuid())) {
                newState = m_layerComposition->m_visibilityMap[node->uuid()];
            }
            if(node->visible() != newState) {
                node->setVisible(m_layerComposition->m_visibilityMap[node->uuid()]);
                node->setDirty();
            }
        }
        
        return true;
    }
private:
    KisLayerComposition* m_layerComposition;
    Mode m_mode;
};

KisLayerComposition::KisLayerComposition(KisImageWSP image, const QString& name): m_image(image), m_name(name)
{

}

KisLayerComposition::~KisLayerComposition()
{

}

QString KisLayerComposition::name()
{
    return m_name;
}

void KisLayerComposition::store()
{
    if(m_image.isNull()) {
        return;
    }
    KisCompositionVisitor visitor(this, KisCompositionVisitor::STORE);
    m_image->rootLayer()->accept(visitor);
}

void KisLayerComposition::apply()
{
    if(m_image.isNull()) {
        return;
    }
    KisCompositionVisitor visitor(this, KisCompositionVisitor::APPLY);
    m_image->rootLayer()->accept(visitor);
}

void KisLayerComposition::setVisible(QUuid id, bool visible)
{
    m_visibilityMap[id] = visible;
}

void KisLayerComposition::save(QDomDocument& doc, QDomElement& element)
{
    QDomElement compositionElement = doc.createElement("composition");
    compositionElement.setAttribute("name", m_name);
    QMapIterator<QUuid, bool> iter(m_visibilityMap);
    while (iter.hasNext()) {
        iter.next();
        QDomElement valueElement = doc.createElement("value");
        valueElement.setAttribute("uuid", iter.key().toString());
        valueElement.setAttribute("visible", iter.value());
        compositionElement.appendChild(valueElement);
    }
    element.appendChild(compositionElement);
}
