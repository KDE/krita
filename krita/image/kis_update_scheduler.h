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

#ifndef __KIS_UPDATE_SCHEDULER_H
#define __KIS_UPDATE_SCHEDULER_H

#include <QObject>
#include "krita_export.h"
#include "kis_types.h"

#include "kis_image_interfaces.h"

class QRect;
class KoProgressProxy;
class KisProjectionUpdateListener;


class KRITAIMAGE_EXPORT KisUpdateScheduler : public QObject, public KisStrokesFacade
{
    Q_OBJECT

public:
    KisUpdateScheduler(KisProjectionUpdateListener *projectionUpdateListener);
    virtual ~KisUpdateScheduler();

    /**
     * Sets the proxy that is going to be notified about the progress
     * of processing of the queues. If you want to switch the proxy
     * on runtime, you should do it under the lock held.
     *
     * \see lock(), unlock()
     */
    void setProgressProxy(KoProgressProxy *progressProxy);

    /**
     * Blocks processing of the queues.
     * The function will wait until all the executing jobs
     * are finished.
     * NOTE: you may add new jobs while the block held, but they
     * will be delayed until unlock() is called.
     *
     * \see unlock()
     */
    void lock();

    /**
     * Unblocks the process and calls processQueues()
     *
     * \see processQueues()
     */
    void unlock();

    /**
     * Called when it is necessary to reread configuration
     */
    void updateSettings();

    /**
     * Waits until all the running jobs are finished.
     *
     * If some other thread adds jobs in parallel, then you may
     * wait forever. If you you don't want it, consider lock() instead.
     *
     * \see lock()
     */
    void waitForDone();

    /**
     * Waits until the queues become empty, then blocks the processing.
     * To unblock processing you should use unlock().
     *
     * If some other thread adds jobs in parallel, then you may
     * wait forever. If you you don't want it, consider lock() instead.
     *
     * \see unlock(), lock()
     */
    void barrierLock();


    /**
     * Works like barrier lock, but returns false immediately if barrierLock
     * can't be acquired.
     *
     * \see barrierLock()
     */
    bool tryBarrierLock();

    /**
     * Blocks all the updates from execution. It doesn't affect
     * strokes execution in any way. This tipe of lock is supposed
     * to be held by the strokes themselves when they need a short
     * access to some parts of the projection of the image.
     * From all the other places you should use usual lock()/unlock()
     * methods
     *
     * \see lock(), unlock()
     */
    void blockUpdates();

    /**
     * Unblocks updates from execution previously locked by blockUpdates()
     *
     * \see blockUpdates()
     */
    void unblockUpdates();

    void updateProjection(KisNodeSP node, const QRect& rc, const QRect &cropRect);
    void fullRefreshAsync(KisNodeSP root, const QRect& rc, const QRect &cropRect);
    void fullRefresh(KisNodeSP root, const QRect& rc, const QRect &cropRect);

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy);
    void addJob(KisStrokeId id, KisStrokeJobData *data);
    void endStroke(KisStrokeId id);
    bool cancelStroke(KisStrokeId id);

protected:
    // Trivial constructor for testing support
    KisUpdateScheduler();
    void connectSignals();
    void processQueues();

private slots:
    void continueUpdate(const QRect &rect);
    void doSomeUsefulWork();
    void spareThreadAppeared();

private:
    friend class UpdatesBlockTester;
    bool haveUpdatesRunning();
    void tryProcessUpdatesQueue();
    void wakeUpWaitingThreads();

    void progressUpdate();
    void progressNotifyJobDone();

protected:
    struct Private;
    Private * const m_d;
};


class KisTestableUpdaterContext;
class KisTestableSimpleUpdateQueue;

class KRITAIMAGE_EXPORT KisTestableUpdateScheduler : public KisUpdateScheduler
{
public:
    KisTestableUpdateScheduler(KisProjectionUpdateListener *projectionUpdateListener,
                               qint32 threadCount);

    KisTestableUpdaterContext* updaterContext();
    KisTestableSimpleUpdateQueue* updateQueue();
    using KisUpdateScheduler::processQueues;
};

#endif /* __KIS_UPDATE_SCHEDULER_H */

