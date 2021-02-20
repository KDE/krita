/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
