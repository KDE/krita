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

#include "kis_abstract_update_queue.h"

KisAbstractUpdateQueue::KisAbstractUpdateQueue()
    : m_processingBlocked(false)
{
}

KisAbstractUpdateQueue::~KisAbstractUpdateQueue()
{

}

void KisAbstractUpdateQueue::processQueue(KisUpdaterContext &updaterContext)
{
    if(m_processingBlocked) return;

    updaterContext.lock();

    while(updaterContext.hasSpareThread()) {
        if(!processOneJob(updaterContext))
            break;
    }

    updaterContext.unlock();
}

void KisAbstractUpdateQueue::startProcessing(KisUpdaterContext &updaterContext)
{
    m_processingBlocked = false;
    processQueue(updaterContext);
}

void KisAbstractUpdateQueue::blockProcessing(KisUpdaterContext &updaterContext)
{
    m_processingBlocked = true;
    updaterContext.waitForDone();
}

void KisAbstractUpdateQueue::executeJobSync(KisBaseRectsWalkerSP walker,
                                            KisUpdaterContext &updaterContext)
{
    updaterContext.lock();

    blockProcessing(updaterContext);

    Q_ASSERT(updaterContext.isJobAllowed(walker));
    updaterContext.addJob(walker);
    updaterContext.waitForDone();

    startProcessing(updaterContext);

    updaterContext.unlock();
}

