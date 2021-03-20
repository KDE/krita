/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_COUNT_VISITOR
#define KIS_COUNT_VISITOR

#include "kritaimage_export.h"

#include <KoProperties.h>

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
/**
 * The count visitor traverses the node stack for nodes that conform
 * to certain properties. You can set the types of nodes to count and
 * add a list of properties to check. The children of nodes that are
 * not counted will be checked and counted if they conform to the
 * requirements.
 */
class KRITAIMAGE_EXPORT KisCountVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisCountVisitor(const QStringList & nodeTypes, const KoProperties & properties)
        : m_nodeTypes(nodeTypes)
        , m_properties(properties)
        , m_count(0) {
    }

    quint32 count() {
        return m_count;
    }

    bool visit(KisNode* node) override {
        return check(node);
    }

    bool visit(KisPaintLayer *layer) override {
        return check(layer);
    }

    bool visit(KisGroupLayer *layer) override {
        return check(layer);
    }


    bool visit(KisAdjustmentLayer *layer) override {
        return check(layer);
    }


    bool visit(KisExternalLayer *layer) override {
        return check(layer);
    }


    bool visit(KisCloneLayer *layer) override {
        return check(layer);
    }


    bool visit(KisFilterMask *mask) override {
        return check(mask);
    }

    bool visit(KisTransformMask *mask) override {
        return check(mask);
    }

    bool visit(KisTransparencyMask *mask) override {
        return check(mask);
    }


    bool visit(KisGeneratorLayer * layer) override {
        return check(layer);
    }

    bool visit(KisSelectionMask* mask) override {
        return check(mask);
    }

    bool visit(KisColorizeMask* mask) override {
        return check(mask);
    }

private:

    bool inList(KisNode* node);
    bool check(KisNode * node);

    const QStringList m_nodeTypes;
    const KoProperties m_properties;
    quint32 m_count;
};
#endif
