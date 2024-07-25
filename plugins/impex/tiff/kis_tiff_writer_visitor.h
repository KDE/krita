/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TIFF_WRITER_VISITOR_H
#define KIS_TIFF_WRITER_VISITOR_H

#include <tiffio.h>

#include <array>

#include <generator/kis_generator_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_annotation.h>
#include <kis_clone_layer.h>
#include <kis_external_layer_iface.h>
#include <kis_generator_layer.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_node_visitor.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <kis_types.h>

#include "kis_tiff_base_writer.h"

struct KisTIFFOptions;

/**
   @author Cyrille Berger <cberger@cberger.net>
*/
class KisTIFFWriterVisitor : public KisNodeVisitor, protected KisTIFFBaseWriter
{
public:

    using KisNodeVisitor::visit;

    KisTIFFWriterVisitor(TIFF*image, KisTIFFOptions* options);
    ~KisTIFFWriterVisitor() override;

public:

    bool visit(KisNode*) override {
        return true;
    }

    bool visit(KisPaintLayer *layer) override {
        return saveLayerProjection(layer);
    }

    bool visit(KisGroupLayer *layer) override {
        dbgFile << "Visiting on grouplayer" << layer->name() << "";
        return visitAll(layer, true);
    }

    bool visit(KisGeneratorLayer *layer) override {
        return saveLayerProjection(layer);
    }

    bool visit(KisCloneLayer *layer) override {
        return saveLayerProjection(layer);
    }

    bool visit(KisExternalLayer *layer) override {
        return saveLayerProjection(layer);
    }

    bool visit(KisAdjustmentLayer *layer) override {
        return saveLayerProjection(layer);
    }

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


private:
    bool saveLayerProjection(KisLayer *);
};

#endif
