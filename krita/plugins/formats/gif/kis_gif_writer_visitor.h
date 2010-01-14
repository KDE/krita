/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_GIF_WRITER_VISITOR_H
#define KIS_GIF_WRITER_VISITOR_H

#include <QImage>

#include <kis_node_visitor.h>
#include <kis_types.h>

class KisGifWriterVisitor : public KisNodeVisitor
{
public:

    using KisNodeVisitor::visit;

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
    bool visit(KisAdjustmentLayer*) {
        return true;
    }

private:
    // Add some options?
    bool saveLayerProjection(KisLayer* layer);

    friend class GifConverter;

    QVector<QImage> m_layers;

};

#endif
