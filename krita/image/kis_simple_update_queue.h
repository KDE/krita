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

#ifndef __KIS_SIMPLE_UPDATE_QUEUE_H
#define __KIS_SIMPLE_UPDATE_QUEUE_H

#include "kis_abstract_update_queue.h"

#include <QMutex>

typedef QList<KisBaseRectsWalkerSP> KisWalkersList;
typedef QListIterator<KisBaseRectsWalkerSP> KisWalkersListIterator;
typedef QMutableListIterator<KisBaseRectsWalkerSP> KisMutableWalkersListIterator;

class KRITAIMAGE_EXPORT KisSimpleUpdateQueue : public KisAbstractUpdateQueue
{
public:
    KisSimpleUpdateQueue();
    ~KisSimpleUpdateQueue();

    void addJob(KisBaseRectsWalkerSP walker);
    void optimize();

protected:
    bool processOneJob(KisUpdaterContext &updaterContext);

protected:
    QMutex m_lock;
    KisWalkersList m_list;
};

class KRITAIMAGE_EXPORT KisTestableSimpleUpdateQueue : public KisSimpleUpdateQueue
{
public:
    KisWalkersList& getWalkersList();
};

#endif /* __KIS_SIMPLE_UPDATE_QUEUE_H */

