/*
 *  copyright (c) 2005 Boudewijn Rempt
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

#ifndef KIS_THREAD_
#define KIS_THREAD_

#include <qthread.h>
#include <ksharedptr.h>

/**
 * A KisThread is a QThread that can be set in the canceled state.
 * Lengthy operations initiated in run() should regularly read the
 * canceled state and stop when it's set to true
 */
class KisThread : public QThread {
    
public:

    /**
     * Create a new KisThread with the canceled state set to false
     */
    KisThread() : QThread(), m_canceled(false) {};

    /**
     * Request the thread to cancel at the first opportunity. Note
     * that the owner of the thread is responsible for restoring the
     * previous state of paint devices etc, the thread itself just stops
     * as soon as possible.
     */
    virtual void cancel() { m_canceled = true; }
    virtual bool isCanceled() { return m_canceled; }

    void runDirectly() { run(); }
    
protected:
    
    bool m_canceled;

};


#endif
