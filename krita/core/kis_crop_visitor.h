/*
 *  copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_CROP_VISITOR_H_
#define KIS_CROP_VISITOR_H_

#include "qrect.h"

#include "klocale.h"

#include "kis_layer_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_transaction.h"

class KisProgressDisplayInterface;
class KisFilterStrategy;

class KisCropVisitor : public KisLayerVisitor {

public:

    KisCropVisitor( const QRect & rc) 
        : KisLayerVisitor()
        , m_rect(rc)
    {
    }

    virtual ~KisCropVisitor()
    {
    }

    /**
     * Crops the specified layer and adds the undo information to the undo adapter of the
     * layer's image.
     */
    bool visit(KisPaintLayer *layer) 
    {
        KisPaintDeviceImplSP dev = layer->paintDevice();

        KisSelectedTransaction * t = new KisSelectedTransaction(i18n("Crop"), dev.data());
        Q_CHECK_PTR(t);

        dev -> crop(m_rect);

        if (layer->undoAdapter()) {
            layer->undoAdapter()->addCommand(t);

            KNamedCommand * cmd = dev -> moveCommand(layer->x() - m_rect.x(), layer->y() - m_rect.y());
            layer->undoAdapter()->addCommand(cmd);
        }

        return true;
    };

    bool visit(KisGroupLayer *layer)
    {
        //KisCropVisitor visitor (m_img, m_sx, m_sy, m_progress, m_filterStrategy);

        KisLayerSP child = layer->firstChild();
        while (child) {
            child->accept(*this);
            child = child->nextSibling();
        }

        return true;
    };

    bool visit(KisPartLayer */*layer*/)
    {
        return true;
    };

private:
    QRect m_rect;
};


#endif
