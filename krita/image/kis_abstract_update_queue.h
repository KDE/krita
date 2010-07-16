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

#ifndef __KIS_ABSTRACT_UPDATE_QUEUE_H
#define __KIS_ABSTRACT_UPDATE_QUEUE_H

#include "kis_updater_context.h"

/**
 * Base class for storing update jobs.
 * It provides interface for appending/executing jobs,
 * optimizing execution process by merging several jobs and
 * temporarily blocking process of execution the queue
 */

class KRITAIMAGE_EXPORT KisAbstractUpdateQueue
{
public:
    KisAbstractUpdateQueue();
    virtual ~KisAbstractUpdateQueue();

    /**
     * Unblocks the process and calls processQueue()
     *
     * \see processQueue()
     */
    void startProcessing(KisUpdaterContext &updaterContext);

    /**
     * Blocks processing of the queue.
     * The function will wait until all the executing jobs
     * are finished.
     * NOTE: you may add new jobs while the block held, but they
     * will be delayed until startProcessing() is called.
     *
     * \see startProcessing()
     */
    void blockProcessing(KisUpdaterContext &updaterContext);

    /**
     * Execute queued jobs in the updateContext
     * If the processing is blocked, the function returns
     * immediately doing nothing.
     *
     * \see blockProcessing()
     */
    void processQueue(KisUpdaterContext &updaterContext);


    /**
     * Execute a job synchronously. It will first wait until all
     * running jobs are finished, then will execute the given
     * job and return.
     */
    void executeJobSync(KisBaseRectsWalkerSP walker,
                        KisUpdaterContext &updaterContext);

    /**
     * Append a new job into the queue.
     * Must be overriden by the descendants.
     */
    virtual void addJob(KisNodeSP node, const QRect& rc, const QRect& cropRect) = 0;

    /**
     * Optimize the queue's jobs.
     * Must be overriden by the descendants.
     */
    virtual void optimize() = 0;

    /**
     * Aren't there any jobs left?
     */
    virtual bool isEmpty() = 0;

    /**
     * The update scheduler will tell us when
     * it is nessesary to reread configuration
     */
    virtual void updateSettings() = 0;

protected:
    /**
     * Process one job of the queue.
     * Must be overriden by the descendants.
     */
    virtual bool processOneJob(KisUpdaterContext &updaterContext) = 0;

private:
    bool m_processingBlocked;
};

#endif /* __KIS_ABSTRACT_UPDATE_QUEUE_H */

