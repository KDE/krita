/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_EXIF_INFO_VISITOR_H
#define KIS_EXIF_INFO_VISITOR_H

#include <kis_node_visitor.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

class KisExifInfoVisitor : public KisNodeVisitor
{
public:

    KisExifInfoVisitor() :
            m_exifInfo(0),
            m_countPaintLayer(0) { }
public:


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
    bool visit(KisGeneratorLayer*) {
        return true;
    }
    bool visit(KisAdjustmentLayer*) {
        return true;
    }

    bool visit(KisPaintLayer* layer) {
        m_countPaintLayer++;
        if (!layer->metaData()->empty()) {
            m_exifInfo = layer->metaData();
        }
        return true;
    }


    bool visit(KisGroupLayer* layer) {
        dbgFile << "Visiting on grouplayer" << layer->name() << "";
        return visitAll(layer, true);
    }


public:
    inline uint countPaintLayer() {
        return m_countPaintLayer;
    }
    inline KisMetaData::Store* exifInfo() {
        return m_exifInfo;
    }
private:
    KisMetaData::Store* m_exifInfo;
    uint m_countPaintLayer;
};

#endif
