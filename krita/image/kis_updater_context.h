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

#ifndef __KIS_UPDATER_CONTEXT_H
#define __KIS_UPDATER_CONTEXT_H

#include <QMutex>
#include <QRunnable>

#include "kis_merge_walker.h"
#include "kis_async_merger.h"

class KisUpdateJobItem :  public QObject, public QRunnable
{
    Q_OBJECT

public:
    KisUpdateJobItem() {
        setAutoDelete(false);
    }

    void run() {
        m_merger.startMerge(*m_walker);

        setWalker(0);
        emit sigJobFinished();
    }

    inline void setWalker(KisMergeWalkerSP walker) {
        m_walker = walker;
    }

    inline KisMergeWalkerSP walker() const {
        return m_walker;
    }

    inline bool isRunning() const {
        return m_walker;
    }

signals:
    void sigJobFinished();

private:
    KisMergeWalkerSP m_walker;
    KisAsyncMerger m_merger;
};


class KRITAIMAGE_EXPORT KisUpdaterContext : public QObject
{
    Q_OBJECT

public:
    KisUpdaterContext();
    ~KisUpdaterContext();

    /**
     * Checks whether the walker intersects with any
     * of currently executing walkers. If it does,
     * it is not allowed to go in. It should be called
     * with the lock held.
     *
     * \see lock()
     */
    bool isJobAllowed(KisMergeWalkerSP walker);

    /**
     * Registers the job and starts executing it.
     * The caller must ensure that the context is locked
     * with lock(), job is allowed with isWalkerAllowed()
     *
     * \see lock()
     * \see isWalkerAllowed()
     * \return true if the walker was successfully added and false
     * if there is no spare thread present
     */
    bool addJob(KisMergeWalkerSP walker);

    /**
     * Locks the context to guarantee an exclusive access
     * to the context
     */
    void lock();

    /**
     * Unlocks the context
     *
     * \see lock()
     */
    void unlock();

signals:
    void wantSomeWork();

protected slots:
    void slotJobFinished();

protected:
    static bool walkersIntersect(KisMergeWalkerSP walker1,
                          KisMergeWalkerSP walker2);
    qint32 findSpareThread();

protected:
    QMutex m_lock;
    QVector<KisUpdateJobItem*> m_jobs;
    QThreadPool m_threadPool;
};

class KRITAIMAGE_EXPORT KisTestableUpdaterContext : public KisUpdaterContext
{
public:
    /**
     * The only difference - it doesn't start execution
     * of the job
     */
    bool addJob(KisMergeWalkerSP walker);
};


#endif /* __KIS_UPDATER_CONTEXT_H */

