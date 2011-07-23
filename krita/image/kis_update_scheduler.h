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

#include "kis_stroke_strategy.h"
#include "kis_stroke_job_strategy.h"

class QRect;


class KRITAIMAGE_EXPORT KisUpdateScheduler : public QObject
{
    Q_OBJECT

public:
    KisUpdateScheduler(KisImageWSP image);
    virtual ~KisUpdateScheduler();

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
     * Waits until all the running jobs are finished. If some other thread
     * adds jobs in parallel, then you may wait forever. If you you don't
     * want it, consider lock() instead
     *
     * \see lock()
     */
    void waitForDone();

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
    void connectImage(KisImageWSP image);
    void processQueues();

private slots:
    void doSomeUsefulWork();
    void spareThreadAppeared();

protected:
    class Private;
    Private * const m_d;
};


class KisTestableUpdaterContext;
class KisTestableSimpleUpdateQueue;

class KRITAIMAGE_EXPORT KisTestableUpdateScheduler : public KisUpdateScheduler
{
public:
    KisTestableUpdateScheduler(KisImageWSP image, qint32 threadCount);

    KisTestableUpdaterContext* updaterContext();
    KisTestableSimpleUpdateQueue* updateQueue();
    using KisUpdateScheduler::processQueues;
};

#endif /* __KIS_UPDATE_SCHEDULER_H */

