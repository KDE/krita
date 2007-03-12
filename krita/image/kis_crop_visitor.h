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

#include "kis_doc2.h"
#include "kis_layer_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transaction.h"
#include "kis_selected_transaction.h"
#include "kis_external_layer_iface.h"
#include "kis_undo_adapter.h"
#include "kis_layer_commands.h"

class KisProgressDisplayInterface;
class KisFilterStrategy;

class KisCropVisitor : public KisLayerVisitor {

public:

    KisCropVisitor( const QRect & rc, bool movelayers = true)
        : KisLayerVisitor()
        , m_rect(rc), m_movelayers(movelayers)
    {
    }

    virtual ~KisCropVisitor()
    {
    }

    bool visit( KisExternalLayer * )
        {
            return true;
        }

    /**
     * Crops the specified layer and adds the undo information to the undo adapter of the
     * layer's image.
     */
    bool visit(KisPaintLayer *layer)
    {
        KisPaintDeviceSP dev = layer->paintDevice();
        KisUndoAdapter* undoAdapter = layer->image()->undoAdapter();

        KisSelectedTransaction * t = 0;
        if (undoAdapter && undoAdapter->undo())
            t = new KisSelectedTransaction(i18n("Crop"), dev);

        dev->crop(m_rect);

        if (undoAdapter && undoAdapter->undo()) {
            undoAdapter->addCommand(t);
        }

        if(m_movelayers) {
            if(undoAdapter && undoAdapter->undo()) {
                QPoint oldPos(layer->x(), layer->y());
                QPoint newPos(layer->x() - m_rect.x(), layer->y() - m_rect.y());
                QUndoCommand * cmd = new KisLayerMoveCommand(layer, oldPos, newPos);
                undoAdapter->addCommand(cmd);
            }
        }
        layer->setDirty();
        return true;
    };

    bool visit(KisGroupLayer *layer)
    {
	layer->resetProjection();

        KisLayerSP child = layer->firstChild();
        while (child) {
            child->accept(*this);
            child = child->nextSibling();
        }
        layer->setDirty();
        return true;
    };

    virtual bool visit(KisAdjustmentLayer* layer)
    {
        layer->resetCache();
        return true;
    }


private:
    QRect m_rect;
    bool m_movelayers;
};


#endif
