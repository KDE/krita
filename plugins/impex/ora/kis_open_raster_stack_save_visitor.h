/*
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_OPEN_RASTER_STACK_SAVE_VISITOR_H_
#define KIS_OPEN_RASTER_STACK_SAVE_VISITOR_H_

#include <QSet>

#include "kis_global.h"
#include "kis_types.h"

#include "kis_node_visitor.h"
#include "kis_layer.h"

class KisOpenRasterSaveContext;

class KisAdjustmentLayer;
class KisGroupLayer;
class KisPaintLayer;
class KisGeneratorLayer;

class QDomElement;

class KisOpenRasterStackSaveVisitor : public KisNodeVisitor
{
public:
    KisOpenRasterStackSaveVisitor(KisOpenRasterSaveContext*, vKisNodeSP activeNodes);
    ~KisOpenRasterStackSaveVisitor() override;

    using KisNodeVisitor::visit;

public:
    bool visit(KisPaintLayer *layer) override;
    bool visit(KisGroupLayer *layer) override;
    bool visit(KisAdjustmentLayer *layer) override;
    bool visit(KisGeneratorLayer * layer) override;

    bool visit(KisNode*) override {
        return true;
    }

    bool visit(KisCloneLayer*) override;

    bool visit(KisFilterMask*) override {
        return true;
    }
    bool visit(KisTransformMask*) override {
        return true;
    }
    bool visit(KisTransparencyMask*) override {
        return true;
    }
    bool visit(KisSelectionMask*) override {
        return true;
    }
    bool visit(KisColorizeMask*) override {
        return true;
    }

    bool visit(KisExternalLayer*) override;

private:
    bool saveLayer(KisLayer *layer);
    void saveLayerInfo(QDomElement& elt, KisLayer* layer);
    struct Private;
    Private* const d;
};


#endif // KIS_LAYER_VISITOR_H_

