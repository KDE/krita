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
#include "kis_transaction.h"
#include "kis_selected_transaction.h"
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

    bool visit(KisExternalLayer *) {
        return true;
    }

    /**
     * Crops the specified layer and adds the undo information to the undo adapter of the
     * layer's image.
     */
    bool visit(KisPaintLayer *layer) {
        return cropPaintDeviceLayer(layer);
    }

    bool visit(KisGroupLayer *layer) {
        layer->resetCache();

        KisNodeSP child = layer->firstChild();
        while (child) {
            child->accept(*this);
            child = child->nextSibling();
        }
        layer->setDirty();
        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer) {
        // XXX: crop the selection?
        layer->resetCache();
        return true;
    }

    virtual bool visit(KisGeneratorLayer * layer) {
        return cropPaintDeviceLayer(layer);
    }

    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer*) {
        return false;
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

    bool cropPaintDeviceLayer(KisLayer * layer) {

        KisPaintDeviceSP dev = layer->paintDevice();
        KisUndoAdapter* undoAdapter = layer->image()->undoAdapter();

        KisSelectedTransaction * t = 0;
        if (undoAdapter && undoAdapter->undo())
            t = new KisSelectedTransaction(i18n("Crop"), layer);

        layer->setDirty();
        dev->crop(m_rect);

        if (undoAdapter && undoAdapter->undo()) {
            undoAdapter->addCommand(t);
        }

        if (m_movelayers) {
            if (undoAdapter && undoAdapter->undo()) {
                QPoint oldPos(layer->x(), layer->y());
                QPoint newPos(layer->x() - m_rect.x(), layer->y() - m_rect.y());
                QUndoCommand * cmd = new KisNodeMoveCommand(layer, oldPos, newPos);
                undoAdapter->addCommand(cmd);
            }
        }
        return true;
    }

    QRect m_rect;
    bool m_movelayers;
};


#endif
