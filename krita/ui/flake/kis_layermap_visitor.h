/*
 *  Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KIS_LAYERMAP_VISITOR_H_
#define KIS_LAYERMAP_VISITOR_H_

#include <KoShape.h>
#include <KoShapeContainer.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_external_layer_iface.h"

/**
 * Creates the right layershape for all layers and puts them in the
 * right order
 */
class KisLayerMapVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisLayerMapVisitor(QMap<KisNodeSP, KoShape*> & nodeMap);
    virtual ~KisLayerMapVisitor() {}

    QMap<KisNodeSP, KoShape*> & layerMap();

public:

    bool visit(KisNode*) {
        return true;
    }

    bool visit(KisExternalLayer * layer);

    bool visit(KisPaintLayer *layer);

    bool visit(KisGroupLayer *layer);

    bool visit(KisAdjustmentLayer *layer);

    bool visit(KisCloneLayer *layer);

    bool visit(KisFilterMask *mask);

    bool visit(KisTransparencyMask *mask);

    bool visit(KisTransformationMask *mask);

    bool visit(KisSelectionMask *mask);

    bool visit(KisGeneratorLayer * layer);

private:

    bool visitLeafNodeLayer(KisLayer * layer);
    bool visitMask(KisMask * mask);

    QMap<KisNodeSP, KoShape*> m_nodeMap;
};


#endif // KIS_LAYERMAP_VISITOR_H_

