/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_updater_context.h"

#include <QThread>
#include <QThreadPool>

KisUpdaterContext::KisUpdaterContext()
{
    qint32 idealThreadCount = QThread::idealThreadCount();
    idealThreadCount = idealThreadCount > 0 ? idealThreadCount : 1;

    m_jobs.resize(idealThreadCount);
    for(qint32 i = 0; i < m_jobs.size(); i++) {
        m_jobs[i] = new KisUpdateJobItem();
        connect(m_jobs[i], SIGNAL(sigJobFinished()),
                SLOT(slotJobFinished()), Qt::DirectConnection);
    }
}

KisUpdaterContext::~KisUpdaterContext()
{
    m_threadPool.waitForDone();
    for(qint32 i = 0; i < m_jobs.size(); i++)
        delete m_jobs[i];
}

bool KisUpdaterContext::isJobAllowed(KisMergeWalkerSP walker)
{
    bool intersects = false;

    foreach(const KisUpdateJobItem *item, m_jobs) {
        if(walkersIntersect(walker, item->walker())) {
            intersects = true;
            break;
        }
    }

    return !intersects;
}

bool KisUpdaterContext::addJob(KisMergeWalkerSP walker)
{
    qint32 jobIndex = findSpareThread();
    if(jobIndex < 0) return false;

    m_jobs[jobIndex]->setWalker(walker);
    m_threadPool.start(m_jobs[jobIndex]);

    return true;
}

/**
 * This variant is for use in a testing suite only
 */
bool KisTestableUpdaterContext::addJob(KisMergeWalkerSP walker)
{
    qint32 jobIndex = findSpareThread();
    if(jobIndex < 0) return false;

    m_jobs[jobIndex]->setWalker(walker);
    // HINT: Not calling start() here

    return true;
}

bool KisUpdaterContext::walkersIntersect(KisMergeWalkerSP walker1,
                                        KisMergeWalkerSP walker2)
{
    return (walker1->accessRect().intersects(walker2->changeRect())) ||
        (walker2->accessRect().intersects(walker1->changeRect()));
}

qint32 KisUpdaterContext::findSpareThread()
{
    for(qint32 i=0; i < m_jobs.size(); i++)
        if(!m_jobs[i]->isRunning())
            return i;

    return -1;
}

void KisUpdaterContext::slotJobFinished()
{
    // Be careful. This slot can be called asynchronously without locks.
    emit wantSomeWork();
}

void KisUpdaterContext::lock()
{
    m_lock.lock();
}

void KisUpdaterContext::unlock()
{
    m_lock.unlock();
}
