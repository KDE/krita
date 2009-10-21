/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#ifndef KIS_REFRESH_VISITOR_H_
#define KIS_REFRESH_VISITOR_H_

#include "kis_node_visitor.h"
#include "kis_image.h"

/**
 * Update the projections of all nodes in the image.
 */
class KisRefreshVisitor : public KisNodeVisitor
{

public:

    using KisNodeVisitor::visit;

    KisRefreshVisitor(KisImageWSP image)
            : KisNodeVisitor() {
        m_image = image;
    }

public:

    bool visit(KisExternalLayer *layer) {
        layer->updateProjection(layer->extent());
        return true;
    }

    bool visit(KisGeneratorLayer * layer) {
        layer->updateProjection(layer->extent());
        return true;
    }

    bool visit(KisPaintLayer *layer) {
        layer->updateProjection(layer->extent());
        return true;
    }

    bool visit(KisAdjustmentLayer* layer) {
        layer->updateProjection(layer->extent());
        return true;
    }

    bool visit(KisCloneLayer * layer) {
        layer->updateProjection(layer->extent());
        return true;
    }


    bool visit(KisGroupLayer *layer) {
        KisNodeSP child = layer->firstChild();
        QRect unitedExtent;


        while (child) {
            child->accept(*this);
            unitedExtent |= child->extent();

            child = child->nextSibling();
        }

        layer->updateProjection(unitedExtent);

        return true;
    }

    bool visit(KisNode*) {
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


private:
    KisImageWSP m_image;
};


#endif /* KIS_REFRESH_VISITOR_H_ */

