/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISSHAREDTHREADPOOLADAPTER_H
#define KISSHAREDTHREADPOOLADAPTER_H

#include <QMutex>
#include <QWaitCondition>

#include <KisSharedRunnable.h>

class QThreadPool;

class KRITAGLOBAL_EXPORT KisSharedThreadPoolAdapter
{
public:
    KisSharedThreadPoolAdapter(QThreadPool *parentPool);
    ~KisSharedThreadPoolAdapter();

    void start(KisSharedRunnable *runnable, int priority = 0);
    bool tryStart(KisSharedRunnable *runnable);

    bool waitForDone(int msecs = -1);

private:
    friend class KisSharedRunnable;
    void notifyJobCompleted();

    KisSharedThreadPoolAdapter(KisSharedThreadPoolAdapter &rhs) = delete;

private:
    QThreadPool *m_parentPool;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    int m_numRunningJobs;
};

#endif // KISSHAREDTHREADPOOLADAPTER_H
