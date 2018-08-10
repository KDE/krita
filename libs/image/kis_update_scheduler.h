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
#include "kritaimage_export.h"
#include "kis_types.h"

#include "kis_image_interfaces.h"
#include "kis_stroke_strategy_factory.h"
#include "kis_strokes_queue_undo_result.h"

class QRect;
class KoProgressProxy;
class KisProjectionUpdateListener;
class KisSpontaneousJob;
class KisPostExecutionUndoAdapter;


class KRITAIMAGE_EXPORT KisUpdateScheduler : public QObject, public KisStrokesFacade
{
    Q_OBJECT

public:
    KisUpdateScheduler(KisProjectionUpdateListener *projectionUpdateListener, QObject *parent = 0);
    ~KisUpdateScheduler() override;

    /**
     * Set the number of threads used by the scheduler
     */
    void setThreadsLimit(int value);

    /**
     * Return the number of threads available to the scheduler
     */
    int threadsLimit() const;

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
    void unlock(bool resetLodLevels = true);

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
     * Tells if there are no strokes or updates are running at the
     * moment. Internally calls to tryBarrierLock(), so it is not O(1).
     */
    bool isIdle();

    /**
     * Blocks all the updates from execution. It doesn't affect
     * strokes execution in any way. This type of lock is supposed
     * to be held by the strokes themselves when they need a short
     * access to some parts of the projection of the image.
     *
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

    void updateProjection(KisNodeSP node, const QVector<QRect> &rects, const QRect &cropRect);
    void updateProjection(KisNodeSP node, const QRect &rc, const QRect &cropRect);
    void updateProjectionNoFilthy(KisNodeSP node, const QRect& rc, const QRect &cropRect);
    void fullRefreshAsync(KisNodeSP root, const QRect& rc, const QRect &cropRect);
    void fullRefresh(KisNodeSP root, const QRect& rc, const QRect &cropRect);
    void addSpontaneousJob(KisSpontaneousJob *spontaneousJob);

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy) override;
    void addJob(KisStrokeId id, KisStrokeJobData *data) override;
    void endStroke(KisStrokeId id) override;
    bool cancelStroke(KisStrokeId id) override;

    /**
     * Sets the desired level of detail on which the strokes should
     * work.  Please note that this configuration will be applied
     * starting from the next stroke. Please also note that this value
     * is not guaranteed to coincide with the one returned by
     * currentLevelOfDetail()
     */
    void setDesiredLevelOfDetail(int lod);

    /**
     * Explicitly start regeneration of LoD planes of all the devices
     * in the image. This call should be performed when the user is idle,
     * just to make the quality of image updates better.
     */
    void explicitRegenerateLevelOfDetail();

    /**
     * Install a factory of a stroke strategy, that will be started
     * every time when the scheduler needs to synchronize LOD caches
     * of all the paint devices of the image.
     */
    void setLod0ToNStrokeStrategyFactory(const KisLodSyncStrokeStrategyFactory &factory);

    /**
     * Install a factory of a stroke strategy, that will be started
     * every time when the scheduler needs to postpone all the updates
     * of the *LOD0* strokes.
     */
    void setSuspendUpdatesStrokeStrategyFactory(const KisSuspendResumeStrategyFactory &factory);

    /**
     * \see setSuspendUpdatesStrokeStrategyFactory()
     */
    void setResumeUpdatesStrokeStrategyFactory(const KisSuspendResumeStrategyFactory &factory);

    KisPostExecutionUndoAdapter* lodNPostExecutionUndoAdapter() const;


    /**
     * tryCancelCurrentStrokeAsync() checks whether there is a
     * *running* stroke (which is being executed at this very moment)
     * which is not still open by the owner (endStroke() or
     * cancelStroke() have already been called) and cancels it.
     *
     * \return true if some stroke has been found and cancelled
     *
     * \note This method is *not* part of KisStrokesFacade! It is too
     *       low level for KisImage.  In KisImage it is combined with
     *       more high level requestStrokeCancellation().
     */
    bool tryCancelCurrentStrokeAsync();

    UndoResult tryUndoLastStrokeAsync();

    bool wrapAroundModeSupported() const;
    int currentLevelOfDetail() const;

    void continueUpdate(const QRect &rect);
    void doSomeUsefulWork();
    void spareThreadAppeared();

protected:
    // Trivial constructor for testing support
    KisUpdateScheduler();
    void connectSignals();
    void processQueues();

protected Q_SLOTS:
    /**
     * Called when it is necessary to reread configuration
     */
    void updateSettings();

private:
    friend class UpdatesBlockTester;
    bool haveUpdatesRunning();
    void tryProcessUpdatesQueue();
    void wakeUpWaitingThreads();

    void progressUpdate();

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

