/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_OPEN_RASTER_STACK_SAVE_VISITOR_H_
#define KIS_OPEN_RASTER_STACK_SAVE_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"

#include "kis_node_visitor.h"

#include <krita_export.h>

class KisOpenRasterSaveContext;

class KisAdjustmentLayer;
class KisGroupLayer;
class KisPaintLayer;
class KisGeneratorLayer;

class QDomElement;

class KRITAUI_EXPORT KisOpenRasterStackSaveVisitor : public KisNodeVisitor
{
public:
    KisOpenRasterStackSaveVisitor(KisOpenRasterSaveContext*);
    virtual ~KisOpenRasterStackSaveVisitor();

    using KisNodeVisitor::visit;

public:
    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer *layer);
    bool visit(KisGeneratorLayer * layer);

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer*) {
        return true;
    }
    bool visit(KisFilterMask*) {
        return true;
    }
    bool visit(KisTransparencyMask*) {
        return true;
    }
    bool visit(KisTransformationMask*) {
        return true;
    }
    bool visit(KisSelectionMask*) {
        return true;
    }
    bool visit(KisExternalLayer*) {
        return true;
    }

private:
    void saveLayerInfo(QDomElement& elt, KisLayer* layer);
    struct Private;
    Private* const d;
};


#endif // KIS_LAYER_VISITOR_H_

