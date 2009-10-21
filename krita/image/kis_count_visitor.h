/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_COUNT_VISITOR
#define KIS_COUNT_VISITOR

#include "krita_export.h"

#include <KoProperties.h>

#include "kis_node_visitor.h"

#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_clone_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_selection_mask.h"
#include "kis_transformation_mask.h"
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

    bool visit(KisNode* node) {
        return check(node);
    }

    bool visit(KisPaintLayer *layer) {
        return check(layer);
    }

    bool visit(KisGroupLayer *layer) {
        return check(layer);
    }


    bool visit(KisAdjustmentLayer *layer) {
        return check(layer);
    }


    bool visit(KisExternalLayer *layer) {
        return check(layer);
    }


    bool visit(KisCloneLayer *layer) {
        return check(layer);
    }


    bool visit(KisFilterMask *mask) {
        return check(mask);
    }


    bool visit(KisTransparencyMask *mask) {
        return check(mask);
    }


    bool visit(KisTransformationMask *mask) {
        return check(mask);
    }

    bool visit(KisGeneratorLayer * layer) {
        return check(layer);
    }

    bool visit(KisSelectionMask* mask) {
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
