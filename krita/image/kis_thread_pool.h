/*
 *  copyright (c) 2006 Boudewijn Rempt
 *
 *  This program is free software; you can distribute it and/or modify
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

#ifndef KIS_THREAD_POOL_
#define KIS_THREAD_POOL_

#include <QThread>
#include <QList>
#include <QMutex>

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
    QList<KisThread *> m_threads;
    QList<KisThread *> m_runningThreads;
    QList<KisThread *> m_oldThreads;
    
    static KisThreadPool * m_singleton;
};


#endif
