/*
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_SHEAR_VISITOR_H_
#define KIS_SHEAR_VISITOR_H_

#include "kis_types.h"
#include "kis_progress_subject.h"
#include "kis_layer_visitor.h"
#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"
#include "kis_undo_adapter.h"
#include "kis_transaction.h"
#include "kis_rotate_visitor.h"

class KisShearVisitor : public KisLayerVisitor {
public:
    KisShearVisitor(double xshear, double yshear, KisProgressDisplayInterface *progress)
        : m_xshear(xshear), m_yshear(yshear), m_progress(progress), m_strategy(0), m_undo(0) {};

    void setStrategy(KisFilterStrategy* strategy) { m_strategy = strategy; }
    void setUndoAdapter(KisUndoAdapter* undo) { m_undo = undo; }
public:
    virtual bool visit(KisPaintLayer* layer) {
        KisPaintDeviceSP dev = layer->paintDevice();
        if(!dev)
            return true;

        KisFilterStrategy* strategy = 0;
        if (m_strategy)
            strategy = m_strategy;
        else
            strategy = new KisMitchellFilterStrategy;

        KisTransaction* t = 0;

        if (m_undo && m_undo->undo())
            t = new KisTransaction("", dev.data());

        //Doesn't do anything, internally transforms x and y shear to 0 each :-///
        //KisTransformWorker w(dev, 1.0, 1.0, m_xshear, m_yshear, 0, 0, 0, m_progress, strategy);
        //w.run();

        KisRotateVisitor v;
        v.visitKisPaintDevice(dev);
        v.shear(m_xshear, m_yshear, m_progress);

        if (m_undo && m_undo->undo())
            m_undo->addCommand(t);

        if (!m_strategy)
            delete strategy;

        layer->setDirty();

        return true;
    }

    virtual bool visit(KisGroupLayer* layer) {
        KisLayerSP child = layer->firstChild();

        while(child)
        {
            child->accept(*this);
            child = child->nextSibling();
        }
        return true;
    }

    virtual bool visit(KisPartLayer*) { return true; }
    virtual bool visit(KisAdjustmentLayer *) { return true; }
private:
    double m_xshear;
    double m_yshear;
    KisProgressDisplayInterface* m_progress;
    KisFilterStrategy* m_strategy;
    KisUndoAdapter* m_undo;
};

#endif // KIS_SHEAR_VISITOR_H_
