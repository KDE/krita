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
#include "kis_layer_visitor.h"
#include "kis_external_layer_iface.h"

#include <kis_layer_shape.h>
#include <kis_shape_layer.h>
#include <kis_layer_container_shape.h>

class KisPaintLayer;
class KisGroupLayer;
class KisAdjustmentLayer;

/**
 * Creates the right layershape for all layers and puts them in the
 * right order
 */
class KisLayerMapVisitor : public KisLayerVisitor {
public:

    /**
     * @param layerMap: the map that maps layers to layer shapes
     */
    KisLayerMapVisitor(QMap<KisLayerSP, KoShape*> & layerMap)
        : m_layerMap( layerMap )
        {
        };
    virtual ~KisLayerMapVisitor() {};

    QMap<KisLayerSP, KoShape*> & layerMap() {
        return m_layerMap;
    }

public:

    bool visit( KisExternalLayer * layer)
        {
//             kDebug(41007) << "KisLayerMap visitor adding external layer: " << layer->name() << endl;
            KisShapeLayer * layerShape = dynamic_cast<KisShapeLayer*>( layer );
            if ( !layerShape ) {
//                 kDebug(41007) << "this external layer is not a shape layer!\n";
                return false;
            }
            m_layerMap[layer] = layerShape;
            return true;
        }

    bool visit(KisPaintLayer *layer)
        {
//             kDebug(41007) << "KisLayerMap visitor adding paint layer: " << layer->name() << endl;

            KoShapeContainer * parent = 0;
            if ( m_layerMap.contains( layer->parent() ) ) {
                parent = static_cast<KoShapeContainer*>( m_layerMap[layer->parent()] );
            }

            KisLayerShape * layerShape = new KisLayerShape( parent, layer );
            m_layerMap[layer] = layerShape;

            return true;
        }

    bool visit(KisGroupLayer *layer)
        {
//             kDebug(41007) << "KisLayerMap visitor adding group layer: " << layer->name() << endl;

            KoShapeContainer * parent = 0;
            if ( m_layerMap.contains( layer->parent() ) ) {
                parent = static_cast<KoShapeContainer*>( m_layerMap[layer->parent()] );
            }

            KisLayerContainerShape * layerContainer = new KisLayerContainerShape(parent, layer);
            m_layerMap[layer] = layerContainer;

            KisLayerSP child = layer->firstChild();
            while (child) {
                child->accept(*this);
                child = child->nextSibling();
            }

            return true;
        }

    bool visit(KisAdjustmentLayer *layer)
        {
//             kDebug(41007) << "KisLayerMap visitor adding adjustment layer: " << layer->name() << endl;

            KoShapeContainer * parent = 0;
            if ( m_layerMap.contains( layer->parent() ) ) {
                parent = static_cast<KoShapeContainer*>( m_layerMap[layer->parent()] );
            }

            KisLayerShape * layerShape = new KisLayerShape( parent, layer );
            m_layerMap[layer] = layerShape;

            return true;
        }

private:

    QMap<KisLayerSP, KoShape*> m_layerMap;
};


#endif // KIS_LAYERMAP_VISITOR_H_

