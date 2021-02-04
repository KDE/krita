/*
 *  SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISNODEACTIVATIONACTIONCREATORVISITOR_H
#define KISNODEACTIVATIONACTIONCREATORVISITOR_H

#include <kis_node_visitor.h>

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

#include <QList>
#include <QAction>

class KActionCollection;
class KisNodeManager;

class KisNodeActivationActionCreatorVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisNodeActivationActionCreatorVisitor(KActionCollection *actionCollection, KisNodeManager *nodeManager);
    ~KisNodeActivationActionCreatorVisitor() {}

    bool visit(KisNode* node) override {
        return createAction(node);
    }

    bool visit(KisPaintLayer *layer) override {
        return createAction(layer);
    }

    bool visit(KisGroupLayer *layer) override {
        return createAction(layer);
    }

    bool visit(KisAdjustmentLayer *layer) override {
        return createAction(layer);
    }

    bool visit(KisExternalLayer *layer) override {
        return createAction(layer);
    }

    bool visit(KisCloneLayer *layer) override {
        return createAction(layer);
    }

    bool visit(KisFilterMask *mask) override {
        return createAction(mask);
    }

    bool visit(KisTransformMask *mask) override {
        return createAction(mask);
    }

    bool visit(KisTransparencyMask *mask) override {
        return createAction(mask);
    }

    bool visit(KisGeneratorLayer * layer) override {
        return createAction(layer);
    }

    bool visit(KisSelectionMask* mask) override {
        return createAction(mask);
    }

    bool visit(KisColorizeMask* mask) override {
        return createAction(mask);
    }

private:

    bool createAction(KisNode *node);

    KisNodeManager *m_nodeManager {0};
    KActionCollection *m_actionCollection {0};

};

#endif // KISNODEACTIVATIONACTIONCREATORVISITOR_H
