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

/**
 * Creates the right layershape for all layers and puts them in the
 * right order
 */
class KisLayerMapVisitor : public KisLayerVisitor {
public:

    /**
     * @param layerMap: the map that maps layers to layer shapes
     */
    KisLayerMapVisitor(QMap<KisLayerSP, KoShape*> & layerMap);
    virtual ~KisLayerMapVisitor() {}

    QMap<KisLayerSP, KoShape*> & layerMap();

public:

    bool visit( KisExternalLayer * layer);

    bool visit(KisPaintLayer *layer);

    bool visit(KisGroupLayer *layer);

    bool visit(KisAdjustmentLayer *layer);
private:

    QMap<KisLayerSP, KoShape*> m_layerMap;
};


#endif // KIS_LAYERMAP_VISITOR_H_

