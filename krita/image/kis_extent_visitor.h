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
#ifndef KIS_EXTENT_VISITOR_H_
#define KIS_EXTENT_VISITOR_H_

#include "kis_node_visitor.h"

#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_clone_layer.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"
#include "kis_transformation_mask.h"

/**
 * The ExtentVisitor determines the total extent of all layers
 * that comprise the image: the union therefore of their extents.
 */
class KisExtentVisitor : public KisNodeVisitor {

public:

    /**
     * @param rc: the extent of the image
     * @param exact: if exact is true, we use the very slow exactBounds
     *               function.
     */
    KisExtentVisitor(QRect rc, bool exact)
        : m_imageRect(rc)
        , m_region(rc)
        , m_exact(exact)
        {
        }
    virtual ~KisExtentVisitor() {}

    QRegion region() { return m_region; }

    bool visit( KisExternalLayer * )
        {
            return true;
        }

    bool visit(KisPaintLayer *layer)
        {
            if (m_exact) {
                m_region = m_region.united(layer->exactBounds());
            }
            else {
                m_region = m_region.united(layer->extent());
            }
            return true;
        }

    bool visit(KisGroupLayer *layer)
        {
            KisNodeSP child = layer->firstChild();
            while (child) {
                child->accept(*this);
                child = child->nextSibling();
            }

            return true;
        }

    bool visit(KisAdjustmentLayer *layer)
        {
            if (m_exact) {
                m_region = m_region.united(layer->exactBounds());
            }
            else {
                m_region = m_region.united(layer->extent());
            }
            return true;
        }

    bool visit(KisCloneLayer *layer)
        {
            if (m_exact) {
                m_region = m_region.united(layer->exactBounds());
            }
            else {
                m_region = m_region.united(layer->extent());
            }
            return true;
        }

private:

    QRect m_imageRect;
    QRegion m_region;
    bool m_exact;

};


#endif // KIS_EXTENT_VISITOR_H_

