/*
 *  copyright (c) 2004, 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef KIS_SCALE_VISITOR_H_
#define KIS_SCALE_VISITOR_H_

#include "klocale.h"

#include "kis_progress_subject.h"
#include "kis_progress_display_interface.h"
#include "kis_thread.h"
#include "kis_layer_visitor.h"
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"

class KisProgressDisplayInterface;
class KisFilterStrategy;

class KisScaleWorker : public KisThread {

    /* Structs for the image rescaling routine */
    class Contrib {
    public:
        Q_INT32 m_pixel;
        double m_weight;
    };

    class ContribList {
    public:
        Q_INT32  n;  //number of contributors
        Contrib *p; //pointer to list of contributions
    };

public:

    KisScaleWorker(KisPaintDevice * dev, double sx, double sy,
                   KisFilterStrategy *filterStrategy)
        : KisThread()
        , m_dev(dev)
        , m_sx(sx)
        , m_sy(sy)
        , m_filterStrategy(filterStrategy) {};

    virtual ~KisScaleWorker() {};

    void run();

private:
    Q_INT32 m_pixelSize;
    KisPaintDevice * m_dev;
    double m_sx, m_sy;
    KisFilterStrategy * m_filterStrategy;


    /**
     * calc_x_contrib()
     *
     * Calculates the filter weights for a single target column.
     * contribX->p must be freed afterwards.
     *
     * Returns -1 if error, 0 otherwise.
     */
    int calcContrib(ContribList *contribX, double cale, double fwidth, int srcwidth, KisFilterStrategy *filterStrategy, Q_INT32 i);

    ContribList * contrib;  //array of contribution lists


};


class KisScaleVisitor : public KisLayerVisitor, KisProgressSubject {

public:

    KisScaleVisitor(KisImageSP img,
                    double sx, 
                    double sy, 
                    KisProgressDisplayInterface *progress, 
                    KisFilterStrategy *filterStrategy) 
        : KisLayerVisitor()
        , m_img(img)
        , m_sx(sx)
        , m_sy(sy)
        , m_progress(progress)
        , m_filterStrategy(filterStrategy)
    {
        if ( progress )
            progress -> setSubject(this, true, true);
        emit notifyProgressStage(i18n("Scaling..."),0);
    }

    virtual ~KisScaleVisitor()
    {
        // Wait for all threads to finish
        KisThread * t;
        int threadcount = m_scalethreads.count();
        int i = 0;
        for ( t = m_scalethreads.first(); t; t = m_scalethreads.next()) {
            //progress info
            if (t) t->wait();
            emit notifyProgress((100 / threadcount) * i);
            ++i;

        }
        emit notifyProgressDone();
        // Delete all threads
        m_scalethreads.setAutoDelete(true);
        m_scalethreads.clear();
    }

    bool visit(KisPaintLayer *layer) 
    {
        // XXX: If all is well, then the image's undoadapter will have started a macro for us
        //      This will break in a more multi-threaded environment
        if (m_img->undoAdapter() && m_img->undoAdapter()->undo()) {
            KisTransaction * cmd = new KisTransaction("", layer->paintDevice());
            m_img->undoAdapter()->addCommand(cmd);
        }

        KisScaleWorker * scaleThread = new KisScaleWorker(layer->paintDevice(),
                                                     m_sx, m_sy, m_filterStrategy);
        m_scalethreads.append(scaleThread);
        scaleThread->start();
        //scaleThread->run();
        layer->setDirty();
        return true;
    }

    bool visit(KisGroupLayer *layer)
    {
        //KisScaleVisitor visitor (m_img, m_sx, m_sy, m_progress, m_filterStrategy);

        // XXX: Maybe faster to scale the projection and do something clever to avoid 
	//      recompositing everything?
	layer->resetProjection(); 
       

        KisLayerSP child = layer->firstChild();
        while (child) {
            child->accept(*this);
            child = child->nextSibling();
        }

        return true;
    }

    bool visit(KisPartLayer */*layer*/)
    {
        return true;
    }

    virtual bool visit(KisAdjustmentLayer* layer)
    {
        KisThread * scaleThread = new KisScaleWorker(layer->selection().data(), m_sx, m_sy, m_filterStrategy);
        m_scalethreads.append(scaleThread);
        scaleThread->start();
        layer->resetCache();
        layer->setDirty();
        return true;
    }
    
    
    // Implement KisProgressSubject
    virtual void cancel() 
    {
        KisThread * t;
        for ( t = m_scalethreads.first(); t; t = m_scalethreads.next()) {
            t->cancel();
        }      
    }
    

private:

    QPtrList<KisThread> m_scalethreads;
    KisImageSP m_img;
    double m_sx;
    double m_sy;
    KisProgressDisplayInterface * m_progress;
    KisFilterStrategy * m_filterStrategy;
};

#endif // KIS_SCALE_VISITOR_H_
