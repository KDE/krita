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
#include "kis_layer_visitor.h"
#include "kis_external_layer_iface.h"

#include <kis_layer_shape.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <kis_layer_container_shape.h>
#include <kis_group_layer.h>
#include <kis_mask_shape.h>
#include <kis_adjustment_layer.h>

KisLayerMapVisitor::KisLayerMapVisitor(QMap<KisLayerSP, KoShape*> & layerMap, QMap<KisMaskSP, KoShape*> & maskMap)
    : m_layerMap( layerMap )
    , m_maskMap( maskMap )
{
}

QMap<KisLayerSP, KoShape*> & KisLayerMapVisitor::layerMap()
{
    return m_layerMap;
}

QMap<KisMaskSP, KoShape*> & KisLayerMapVisitor::maskMap()
{
    return m_maskMap;
}


bool KisLayerMapVisitor::visit( KisExternalLayer * layer)
{
//             kDebug(41007) << "KisLayerMap visitor adding external layer: " << layer->name() << endl;
        KisShapeLayer * layerShape = dynamic_cast<KisShapeLayer*>( layer );
        if ( !layerShape ) {
//                 kDebug(41007) << "this external layer is not a shape layer!\n";
            return false;
        }
        m_layerMap[layer] = layerShape;
        fillMaskMap( layer, layerShape );
        return true;
    }

bool KisLayerMapVisitor::visit(KisPaintLayer *layer)
{
//             kDebug(41007) << "KisLayerMap visitor adding paint layer: " << layer->name() << endl;
    Q_ASSERT( layer->parentLayer() );
    Q_ASSERT( m_layerMap.contains( layer->parentLayer() ) );

    if ( m_layerMap.contains( layer->parentLayer() ) ) {

        KoShapeContainer * parent = static_cast<KoShapeContainer*>( m_layerMap[layer->parentLayer()] );
        KisLayerShape * layerShape = new KisLayerShape( parent, layer );
        m_layerMap[layer] = layerShape;
        fillMaskMap( layer, layerShape );
        return true;
    }
    return false;
}

bool KisLayerMapVisitor::visit(KisGroupLayer *layer)
{
//             kDebug(41007) << "KisLayerMap visitor adding group layer: " << layer->name() << endl;

    KoShapeContainer * parent = 0;
    if ( m_layerMap.contains( layer->parentLayer() ) ) {
        parent = static_cast<KoShapeContainer*>( m_layerMap[layer->parentLayer()] );
    }

    KisLayerContainerShape * layerContainer = new KisLayerContainerShape(parent, layer);
    m_layerMap[layer] = layerContainer;

    KisLayerSP child = layer->firstChild();
    while (child) {
        child->accept(*this);
        child = child->nextSibling();
    }

    fillMaskMap( layer, layerContainer );
    return true;
}

bool KisLayerMapVisitor::visit(KisAdjustmentLayer *layer)
{
//             kDebug(41007) << "KisLayerMap visitor adding adjustment layer: " << layer->name() << endl;

    Q_ASSERT( m_layerMap.contains( layer->parentLayer() ) );

    KoShapeContainer * parent = 0;
    if ( m_layerMap.contains( layer->parentLayer() ) ) {
        parent = static_cast<KoShapeContainer*>( m_layerMap[layer->parentLayer()] );
        KisLayerShape * layerShape = new KisLayerShape( parent, layer );
        m_layerMap[layer] = layerShape;
        fillMaskMap( layer, layerShape );
        return true;
    }

    return false;

}

void KisLayerMapVisitor::fillMaskMap( KisLayerSP layer, KoShapeContainer * container )
{
    if ( !layer->hasEffectMasks() ) return;

    QList<KisMaskSP> masks = layer->effectMasks();
    for ( int i = 0; i < masks.size(); ++i ) {
        KisMaskSP mask = masks.at( i );
        KisMaskShape * maskShape = new KisMaskShape( container, mask );
        container->addChild( maskShape );
        m_maskMap[mask] = maskShape;
    }
}
