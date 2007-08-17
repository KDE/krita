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

#include <kis_layermap_visitor.h>

#include <KoShape.h>
#include <KoShapeContainer.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_external_layer_iface.h"

#include <kis_layer_shape.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <kis_clone_layer.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_transformation_mask.h>
#include <kis_filter_mask.h>
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>

#include "kis_layer_container_shape.h"

#include "kis_mask_shape.h"


KisLayerMapVisitor::KisLayerMapVisitor( QMap<KisNodeSP, KoShape*> & nodeMap )
    : m_nodeMap( nodeMap )
{
}

QMap<KisNodeSP, KoShape*> & KisLayerMapVisitor::layerMap()
{
    return m_nodeMap;
}


bool KisLayerMapVisitor::visit( KisExternalLayer * layer)
{
        KisShapeLayer * layerShape = dynamic_cast<KisShapeLayer*>( layer );
        if ( !layerShape ) {
            return false;
        }

        m_nodeMap[layer] = layerShape;
        visitAll( layer );
        return true;
    }

bool KisLayerMapVisitor::visit(KisPaintLayer *layer)
{
    return visitLeafNodeLayer( layer );
}

bool KisLayerMapVisitor::visit(KisGroupLayer *layer)
{
    KoShapeContainer * parent = 0;
    if ( m_nodeMap.contains( layer->parent() ) ) {
        parent = static_cast<KoShapeContainer*>( m_nodeMap[layer->parent()] );
    }

    KisLayerContainerShape * layerContainer = new KisLayerContainerShape(parent, layer);
    m_nodeMap[layer] = layerContainer;
    visitAll( layer );

    return true;
}

bool KisLayerMapVisitor::visit(KisAdjustmentLayer *layer)
{
    return visitLeafNodeLayer( layer );
}

bool KisLayerMapVisitor::visit(KisCloneLayer *layer)
{
    return visitLeafNodeLayer( layer );
}

bool KisLayerMapVisitor::visit(KisFilterMask *mask)
{
    return visitMask( mask );
}

bool KisLayerMapVisitor::visit(KisTransparencyMask *mask)
{
    return visitMask( mask );
}

bool KisLayerMapVisitor::visit(KisTransformationMask *mask)
{
    return visitMask( mask );
}

bool KisLayerMapVisitor::visit(KisSelectionMask *mask)
{
    return visitMask( mask );
}

bool KisLayerMapVisitor::visitLeafNodeLayer( KisLayer * layer )
{
    // Leaf layers can always have masks as subnodes

    Q_ASSERT( layer->parent() );
    Q_ASSERT( m_nodeMap.contains( layer->parent() ) );

    if ( m_nodeMap.contains( layer->parent() ) ) {
        KoShapeContainer * parent = static_cast<KoShapeContainer*>( m_nodeMap[layer->parent()] );
        KisLayerShape * layerShape = new KisLayerShape( parent, layer );
        m_nodeMap[layer] = layerShape;
        visitAll( layer );
        return true;
    }
    return false;
}

bool KisLayerMapVisitor::visitMask( KisMask * mask )
{
    // Masks cannot be nested

    Q_ASSERT( mask->parent() );
    Q_ASSERT( m_nodeMap.contains( mask->parent() ) );

    if ( m_nodeMap.contains( mask->parent() ) ) {
        KoShapeContainer * parent = static_cast<KoShapeContainer*>( m_nodeMap[mask->parent()] );
        KisMaskShape * maskShape = new KisMaskShape( parent, mask );
        m_nodeMap[mask] = maskShape;
        return true;
    }
    return false;

}
