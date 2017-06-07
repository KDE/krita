/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TIFF_WRITER_VISITOR_H
#define KIS_TIFF_WRITER_VISITOR_H

#include <kis_node_visitor.h>
#include "kis_types.h"

#include <tiffio.h>

struct KisTIFFOptions;

/**
   @author Cyrille Berger <cberger@cberger.net>
*/
class KisTIFFWriterVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

    KisTIFFWriterVisitor(TIFF*image, KisTIFFOptions* options);
    ~KisTIFFWriterVisitor();

public:

    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisGeneratorLayer*);

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer*) {
        return true;
    }
    bool visit(KisFilterMask*) {
        return true;
    }
    bool visit(KisTransformMask*) {
        return true;
    }
    bool visit(KisTransparencyMask*) {
        return true;
    }
    bool visit(KisSelectionMask*) {
        return true;
    }
    bool visit(KisColorizeMask*) {
        return true;
    }
    bool visit(KisExternalLayer*);

    bool visit(KisAdjustmentLayer*) {
        return true;
    }

private:
    inline TIFF* image() {
        return m_image;
    }
    bool copyDataToStrips(KisHLineConstIteratorSP it, tdata_t buff, uint8 depth, uint16 sample_format, uint8 nbcolorssamples, quint8* poses);
    bool saveLayerProjection(KisLayer *);
private:
    TIFF* m_image;
    KisTIFFOptions* m_options;
};

#endif
