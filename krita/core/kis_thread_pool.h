/*
 *  copyright (c) 2006 Boudewijn Rempt
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

#ifndef KIS_THREAD_POOL_
#define KIS_THREAD_POOL_

#include <qthread.h>
#include <qptrlist.h>
#include <qmutex.h>

#include "kis_thread.h"

/**
 * A thread pool starts executing threads some time after they are added,
 * running a maximum number of threads at one time.
 *
 * The pool takes ownership of the threads and _deletes_ them once they
 * have run. This means that you cannot add getters for important data to
 * threads you feed the threadpool. Instead, post the data using a customevent.
 */
class KisThreadPool : public KisThread {

public:

    virtual ~KisThreadPool();

    static KisThreadPool * instance();

    void enqueue(KisThread * thread);
    void dequeue(KisThread * thread);
    
    void run();
    
    
    KisThreadPool();

private:

    KisThreadPool(const KisThreadPool&);
    KisThreadPool operator=(const KisThreadPool&);

    QMutex m_poolMutex;
    int m_numberOfRunningThreads;
    int m_numberOfQueuedThreads;
    int m_maxThreads;
    int m_wait;
    QPtrList<KisThread> m_threads;
    QPtrList<KisThread> m_runningThreads;
    QPtrList<KisThread> m_oldThreads;
    
    static KisThreadPool * m_singleton;
};


#endif
