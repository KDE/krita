/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_CROP_VISITOR_H_
#define KIS_CROP_VISITOR_H_

#include "qrect.h"

#include "klocale.h"

#include "kis_node_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"

#include "kis_transparency_mask.h"
#include "kis_filter_mask.h"
#include "kis_selection_mask.h"

#include "kis_transaction.h"
#include "kis_selection.h"
#include "kis_external_layer_iface.h"
#include "kis_undo_adapter.h"
#include "commands/kis_node_commands.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "generator/kis_generator_layer.h"

/**
 * XXX: crop all masks, too?
 */
class KisCropVisitor : public KisNodeVisitor
{

public:

    using KisNodeVisitor::visit;

    KisCropVisitor(const QRect & rc, bool movelayers = true)
            : KisNodeVisitor()
            , m_rect(rc), m_movelayers(movelayers) {
    }

    virtual ~KisCropVisitor() {
    }

    bool visit(KisExternalLayer *layer) {
        KisUndoAdapter *undoAdapter = layer->image()->undoAdapter();
        KUndo2Command* command = layer->crop(m_rect);
        if (command)
            undoAdapter->addCommand(command);
        return visitAll(layer);
    }

    bool visit(KisPaintLayer *layer) {
        cropPaintDeviceNode(layer);
        return visitAll(layer);
    }

    bool visit(KisGroupLayer *layer) {
        layer->resetCache();
        layer->setDirty();
        return visitAll(layer);
    }

    bool visit(KisAdjustmentLayer* layer) {
        layer->resetCache();
        cropPaintDeviceNode(layer);
        return visitAll(layer);

    }

    bool visit(KisGeneratorLayer * layer) {
        layer->resetCache();
        cropPaintDeviceNode(layer);
        return visitAll(layer);
    }

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer*) {
        return true;
    }
    bool visit(KisFilterMask *mask) {
        return cropPaintDeviceNode((KisNode*)mask);
    }
    bool visit(KisTransparencyMask *mask) {
        return cropPaintDeviceNode(mask);
    }
    bool visit(KisSelectionMask *mask) {
        return cropPaintDeviceNode(mask);
    }

private:
    KisImageWSP getImage(KisNode *node) {
        // temporary hack, our masks do not have a link to image.

        while (node) {
            KisLayer *layer = dynamic_cast<KisLayer*>(node);
            if(layer) {
                return layer->image();
            }
            node = node->parent().data();
        }
        return 0;
    }

    /**
     * Crops the specified layer and adds the undo information
     * to the undo adapter of the layer's image.
     */
    bool cropPaintDeviceNode(KisNode *node) {
        /**
         * TODO: implement actual robust cropping of the selections,
         * including the cropping of vector (!) selection.
         */
        KisImageWSP image = getImage(node);
        KisUndoAdapter *undoAdapter = image->undoAdapter();

        QRect nodeExtent = node->extent();
        KisTransaction transaction(i18n("Crop"), node->paintDevice());
        node->paintDevice()->crop(m_rect);
        transaction.commit(undoAdapter);

        if (m_movelayers) {
            QPoint oldPos(node->x(), node->y());
            QPoint newPos(node->x() - m_rect.x(), node->y() - m_rect.y());
            KUndo2Command *cmd = new KisNodeMoveCommand(node, oldPos, newPos, image);
            undoAdapter->addCommand(cmd);
        }

        node->setDirty(nodeExtent);
        return true;
    }

    QRect m_rect;
    bool m_movelayers;
};


#endif
