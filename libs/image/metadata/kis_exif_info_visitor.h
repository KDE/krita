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
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_filter_registry_model.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>

/**
 * @brief The KisExifInfoVisitor class looks for a layer with metadata.
 *
 * If there is more than one layer with metadata, the metadata provided
 * by the visitor is the metadata associated with the last layer that
 * had metadata on it. Only use the metadata if only one layer with
 * metadata was found.
 *
 * The metadata pointer is OWNED by the layer.
 *
 */
class KisExifInfoVisitor : public KisNodeVisitor
{
public:

    KisExifInfoVisitor() { }

    bool visit(KisNode*) override {
        return true;
    }
    bool visit(KisCloneLayer*) override {
        return true;
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
    bool visit(KisExternalLayer*) override {
        return true;
    }
    bool visit(KisGeneratorLayer*) override {
        return true;
    }
    bool visit(KisAdjustmentLayer*) override {
        return true;
    }

    bool visit(KisPaintLayer* layer) override {
        if (!layer->metaData()->empty()) {
            m_metaDataObjectsEncountered++;
            m_exifInfo = layer->metaData();
        }
        return true;
    }

    bool visit(KisGroupLayer* layer) override {
        dbgMetaData << "Visiting on grouplayer" << layer->name() << "";
        return visitAll(layer, true);
    }

public:
    inline uint metaDataCount()
    {
        dbgImage << "number of layers with metadata" << m_metaDataObjectsEncountered;
        return m_metaDataObjectsEncountered;
    }

    inline KisMetaData::Store* exifInfo()
    {
        return m_exifInfo;
    }
private:
    KisMetaData::Store *m_exifInfo {0};
    int m_metaDataObjectsEncountered {0};
};

#endif
