/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_transform_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "lazybrush/kis_colorize_mask.h"
#include "kis_layer_utils.h"
#include "kis_node_query_path.h"

#include <QDomDocument>

class KisCompositionVisitor : public KisNodeVisitor {
public:
    enum Mode {
        STORE,
        APPLY
    };
    
    KisCompositionVisitor(KisLayerComposition* layerComposition, Mode mode)
        : m_layerComposition(layerComposition)
        , m_mode(mode)
    {        
    }

    bool visit(KisNode* node) override { return process(node); }
    bool visit(KisGroupLayer* layer) override
    { 
        bool result = visitAll(layer);
        if(layer == layer->image()->rootLayer()) {
            return result;
        }        
        return result && process(layer);
    }
    bool visit(KisAdjustmentLayer* layer) override { return process(layer); }
    bool visit(KisPaintLayer* layer) override { return process(layer); }
    bool visit(KisExternalLayer* layer) override { return process(layer); }
    bool visit(KisGeneratorLayer* layer) override { return process(layer); }
    bool visit(KisCloneLayer* layer) override { return process(layer); }
    bool visit(KisFilterMask* mask) override { return process(mask); }
    bool visit(KisTransformMask* mask) override { return process(mask); }
    bool visit(KisTransparencyMask* mask) override { return process(mask); }
    bool visit(KisSelectionMask* mask) override { return process(mask); }
    bool visit(KisColorizeMask* mask) override { return process(mask); }

    bool process(KisNode* node) {
        if (node->isFakeNode()) {
            dbgKrita << "Compositions: Skipping over Fake Node" << node->uuid() << node->name();
            return true;
        }
        if(m_mode == STORE) {
            m_layerComposition->m_visibilityMap[node->uuid()] = node->visible();
            m_layerComposition->m_collapsedMap[node->uuid()] = node->collapsed();
        } else {
            bool newState = false;
            if(m_layerComposition->m_visibilityMap.contains(node->uuid())) {
                newState = m_layerComposition->m_visibilityMap[node->uuid()];
            }
            if(node->visible() != newState) {
                node->setVisible(m_layerComposition->m_visibilityMap[node->uuid()]);
                node->setDirty();
            }
            if(m_layerComposition->m_collapsedMap.contains(node->uuid())) {
                node->setCollapsed(m_layerComposition->m_collapsedMap[node->uuid()]);
            }
        }
        
        return true;
    }
private:
    KisLayerComposition* m_layerComposition;
    Mode m_mode;
};

KisLayerComposition::KisLayerComposition(KisImageWSP image, const QString& name)
    : m_image(image)
    , m_name(name)
    , m_exportEnabled(true)
{

}

KisLayerComposition::~KisLayerComposition()
{

}

KisLayerComposition::KisLayerComposition(const KisLayerComposition &rhs, KisImageWSP otherImage)
    : m_image(otherImage ? otherImage : rhs.m_image),
      m_name(rhs.m_name),
      m_exportEnabled(rhs.m_exportEnabled)
{
    {
        auto it = rhs.m_visibilityMap.constBegin();
        for (; it != rhs.m_visibilityMap.constEnd(); ++it) {
            QUuid nodeUuid = it.key();
            KisNodeSP node = KisLayerUtils::findNodeByUuid(rhs.m_image->root(), nodeUuid);
            if (node) {
                KisNodeQueryPath path = KisNodeQueryPath::absolutePath(node);
                KisNodeSP newNode = path.queryUniqueNode(m_image);
                KIS_ASSERT_RECOVER(newNode) { continue; }

                m_visibilityMap.insert(newNode->uuid(), it.value());
            }
        }
    }

    {
        auto it = rhs.m_collapsedMap.constBegin();
        for (; it != rhs.m_collapsedMap.constEnd(); ++it) {
            QUuid nodeUuid = it.key();
            KisNodeSP node = KisLayerUtils::findNodeByUuid(rhs.m_image->root(), nodeUuid);
            if (node) {
                KisNodeQueryPath path = KisNodeQueryPath::absolutePath(node);
                KisNodeSP newNode = path.queryUniqueNode(m_image);
                KIS_ASSERT_RECOVER(newNode) { continue; }

                m_collapsedMap.insert(newNode->uuid(), it.value());
            }
        }
    }
}

void KisLayerComposition::setName(const QString& name)
{
    m_name = name;
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
    if (m_image.isNull()) {
        return;
    }
    KisCompositionVisitor visitor(this, KisCompositionVisitor::APPLY);
    m_image->rootLayer()->accept(visitor);
}

void KisLayerComposition::setExportEnabled ( bool enabled )
{
    m_exportEnabled = enabled;
}

bool KisLayerComposition::isExportEnabled()
{
    return m_exportEnabled;
}

void KisLayerComposition::setVisible(QUuid id, bool visible)
{
    m_visibilityMap[id] = visible;
}

void KisLayerComposition::setCollapsed ( QUuid id, bool collapsed )
{
    m_collapsedMap[id] = collapsed;
}

void KisLayerComposition::save(QDomDocument& doc, QDomElement& element)
{
    QDomElement compositionElement = doc.createElement("composition");
    compositionElement.setAttribute("name", m_name);
    compositionElement.setAttribute("exportEnabled", m_exportEnabled);
    QMapIterator<QUuid, bool> iter(m_visibilityMap);
    while (iter.hasNext()) {
        iter.next();
        QDomElement valueElement = doc.createElement("value");
        dbgKrita << "uuid" << iter.key().toString() << "visible" <<  iter.value();
        valueElement.setAttribute("uuid", iter.key().toString());
        valueElement.setAttribute("visible", iter.value());
        dbgKrita << "contains" << m_collapsedMap.contains(iter.key());
        if (m_collapsedMap.contains(iter.key())) {
            dbgKrita << "colapsed :" << m_collapsedMap[iter.key()];
            valueElement.setAttribute("collapsed", m_collapsedMap[iter.key()]);
        }
        compositionElement.appendChild(valueElement);
    }
    element.appendChild(compositionElement);
}
