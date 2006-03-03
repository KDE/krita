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

#include "kis_thread_pool.h"
#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

KisThreadPool * KisThreadPool::m_singleton = 0;

KisThreadPool::KisThreadPool()
{
    Q_ASSERT(KisThreadPool::m_singleton == 0);

    KisThreadPool::m_singleton = this;

    KConfig * cfg = KGlobal::config();
    cfg->setGroup("");
    m_maxThreads = cfg->readNumEntry("maxthreads",  10);
    m_numberOfRunningThreads = 0;
    m_numberOfQueuedThreads = 0;
    m_wait = 200;

    start();
}


KisThreadPool::~KisThreadPool()
{
    m_poolMutex.lock();

    m_canceled = true;

    m_runningThreads.setAutoDelete(true);
    m_threads.setAutoDelete(true);
    m_oldThreads.setAutoDelete(true);

    KisThread * t;

    for ( t = m_threads.first(); t; t = m_threads.next()) {
        if (t) {
            t->cancel();
            t->wait();
            m_threads.remove(t);
        }
    }

    for ( t = m_runningThreads.first(); t; t = m_runningThreads.next()) {
        if (t) {
            t->cancel();
            t->wait();
            m_runningThreads.remove(t);
        }
    }

    for ( t = m_oldThreads.first(); t; t = m_oldThreads.next()) {
        if (t) {
            t->cancel();
            t->wait();
            m_runningThreads.remove(t);
        }
    }
    KisThreadPool::m_singleton = 0;
    m_poolMutex.unlock();

}


KisThreadPool * KisThreadPool::instance()
{
    if(KisThreadPool::m_singleton == 0)
    {
        KisThreadPool::m_singleton = new KisThreadPool();
    }
    else {

        if (KisThreadPool::m_singleton->finished()) {
            delete KisThreadPool::m_singleton;
            KisThreadPool::m_singleton = 0;
            KisThreadPool::m_singleton = new KisThreadPool();
        }
    }

    return KisThreadPool::m_singleton;
}

void KisThreadPool::enqueue(KisThread * thread)
{
    m_poolMutex.lock();
    m_threads.append(thread);
    m_numberOfQueuedThreads++;
    m_poolMutex.unlock();
    m_wait = 200;
}


void KisThreadPool::dequeue(KisThread * thread)
{
    KisThread * t = 0;

    m_poolMutex.lock();

    int i = m_threads.findRef(thread);
    if (i >= 0) {
        t = m_threads.take(i);
        m_numberOfQueuedThreads--;
    } else {
        i = m_runningThreads.findRef(thread);
        if (i >= 0) {
            t = m_runningThreads.take(i);
            m_numberOfRunningThreads--;
        }
        else {
            i = m_oldThreads.findRef(thread);
            if (i >= 0) {
                t = m_oldThreads.take(i);
            }
        }
    }

    m_poolMutex.unlock();

    if (t) {
        t->cancel();
        t->wait();
        delete t;
    }

}

void KisThreadPool::run()
{
    int sleeps = 10;

    while(!m_canceled) {
        if (m_numberOfQueuedThreads > 0 && m_numberOfRunningThreads < m_maxThreads) {
            KisThread * thread = 0;
            m_poolMutex.lock();
            if (m_threads.count() > 0) {
                thread = m_threads.take();
                m_numberOfQueuedThreads--;
            }
            if (thread) {
                thread->start();
                m_runningThreads.append(thread);
                m_numberOfRunningThreads++;
            }
            m_poolMutex.unlock();
        }
        else {
            msleep(m_wait);
            m_poolMutex.lock();
            for ( KisThread * t = m_runningThreads.first(); t; t = m_runningThreads.next()) {
                if (t) {
                    if (t->finished()) {
                        m_runningThreads.remove(t);
                        m_numberOfRunningThreads--;
                        m_oldThreads.append(t);
                    }
                }
            }
            m_poolMutex.unlock();
            m_poolMutex.lock();
            if (m_numberOfQueuedThreads == 0 && m_numberOfRunningThreads == 0) {
                sleeps--;
                if (sleeps == 0) {
                    m_poolMutex.unlock();
                    return;
                }
                m_poolMutex.unlock();

            }
            m_poolMutex.unlock();

        }
    }
}
