/*
 *  SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KISRESOURCECOLLECTORVISITOR_H
#define KISRESOURCECOLLECTORVISITOR_H

#include "kritaimage_export.h"

#include "kis_node_visitor.h"

#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_clone_layer.h"
#include "kis_filter_mask.h"
#include "kis_transform_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "lazybrush/kis_colorize_mask.h"
#include "generator/kis_generator_layer.h"

#include <KisGlobalResourcesInterface.h>



class KisResourceCollectorVisitor : public KisNodeVisitor
{

public:

    using KisNodeVisitor::visit;

    KisResourceCollectorVisitor() {
    }

    QList <KoResourceSP> resources() {
        return m_resources;
    }

    bool visit(KisNode* node) override {
        return visitAll(node);;
    }

    bool visit(KisPaintLayer *node) override {
        return visitAll(node);
    }

    bool visit(KisGroupLayer *node) override {
        return visitAll(node);
    }


    bool visit(KisAdjustmentLayer *node) override {
        m_resources << node->filter()->embeddedResources(KisGlobalResourcesInterface::instance());
        m_resources << node->filter()->linkedResources(KisGlobalResourcesInterface::instance());
        return visitAll(node);

    }


    bool visit(KisExternalLayer *node) override {
        return visitAll(node);
    }


    bool visit(KisCloneLayer *node) override {
        return visitAll(node);
    }


    bool visit(KisFilterMask *node) override {
        m_resources << node->filter()->embeddedResources(KisGlobalResourcesInterface::instance());
        m_resources << node->filter()->linkedResources(KisGlobalResourcesInterface::instance());
        return visitAll(node);
    }

    bool visit(KisTransformMask *node) override {
        return visitAll(node);
    }

    bool visit(KisTransparencyMask *node) override {
        return visitAll(node);
    }


    bool visit(KisGeneratorLayer *node) override {
        m_resources << node->filter()->embeddedResources(KisGlobalResourcesInterface::instance());
        m_resources << node->filter()->linkedResources(KisGlobalResourcesInterface::instance());
        return visitAll(node);
    }

    bool visit(KisSelectionMask *node) override {
        return visitAll(node);
    }

    bool visit(KisColorizeMask *node) override {
        return visitAll(node);
    }

private:

    QList<KoResourceSP> m_resources;
};

#endif // KISRESOURCECOLLECTORVISITOR_H
