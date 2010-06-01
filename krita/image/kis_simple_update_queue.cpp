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

#include "kis_simple_update_queue.h"

#include <QMutexLocker>

KisSimpleUpdateQueue::KisSimpleUpdateQueue()
{
}

KisSimpleUpdateQueue::~KisSimpleUpdateQueue()
{
}

bool KisSimpleUpdateQueue::processOneJob(KisUpdaterContext &updaterContext)
{
    QMutexLocker locker(&m_lock);

    KisBaseRectsWalkerSP item;
    KisMutableWalkersListIterator iter(m_list);
    bool jobAdded = false;

    while(iter.hasNext()) {
        item = iter.next();
        if(updaterContext.isJobAllowed(item)) {
            updaterContext.addJob(item);
            iter.remove();
            jobAdded = true;
            break;
        }
    }

    return jobAdded;
}

void KisSimpleUpdateQueue::addJob(KisBaseRectsWalkerSP walker)
{
    QMutexLocker locker(&m_lock);
    m_list.append(walker);
}

void KisSimpleUpdateQueue::optimize()
{
}

KisWalkersList& KisTestableSimpleUpdateQueue::getWalkersList()
{
    return m_list;
}

