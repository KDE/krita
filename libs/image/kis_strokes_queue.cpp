/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_strokes_queue.h"

#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include "kis_stroke.h"
#include "kis_updater_context.h"
#include "kis_stroke_job_strategy.h"
#include "kis_stroke_strategy.h"

typedef QQueue<KisStrokeSP> StrokesQueue;
typedef QQueue<KisStrokeSP>::iterator StrokesQueueIterator;

struct Q_DECL_HIDDEN KisStrokesQueue::Private {
    Private()
        : openedStrokesCounter(0),
          needsExclusiveAccess(false),
          wrapAroundModeSupported(false),
          currentStrokeLoaded(false),
          lodNNeedsSynchronization(true),
          desiredLevelOfDetail(0),
          nextDesiredLevelOfDetail(0) {}

    StrokesQueue strokesQueue;
    int openedStrokesCounter;
    bool needsExclusiveAccess;
    bool wrapAroundModeSupported;
    bool currentStrokeLoaded;

    bool lodNNeedsSynchronization;
    int desiredLevelOfDetail;
    int nextDesiredLevelOfDetail;
    QMutex mutex;
    KisLodSyncStrokeStrategyFactory lod0ToNStrokeStrategyFactory;
    KisSuspendResumeStrategyFactory suspendUpdatesStrokeStrategyFactory;
    KisSuspendResumeStrategyFactory resumeUpdatesStrokeStrategyFactory;

    void cancelForgettableStrokes();
    void startLod0ToNStroke(int levelOfDetail, bool forgettable);

    bool canUseLodN() const;
    StrokesQueueIterator findNewLod0Pos();
    StrokesQueueIterator findNewLodNPos(KisStrokeSP lodN);
    bool shouldWrapInSuspendUpdatesStroke() const;

    void switchDesiredLevelOfDetail(bool forced);
    bool hasUnfinishedStrokes() const;
};


KisStrokesQueue::KisStrokesQueue()
  : m_d(new Private)
{
}

KisStrokesQueue::~KisStrokesQueue()
{
    Q_FOREACH (KisStrokeSP stroke, m_d->strokesQueue) {
        stroke->cancelStroke();
    }

    delete m_d;
}

template <class StrokePair, class StrokesQueue>
typename StrokesQueue::iterator
executeStrokePair(const StrokePair &pair, StrokesQueue &queue, typename StrokesQueue::iterator it, KisStroke::Type type, int levelOfDetail) {
    KisStrokeStrategy *strategy = pair.first;
    QList<KisStrokeJobData*> jobsData = pair.second;

    KisStrokeSP stroke(new KisStroke(strategy, type, levelOfDetail));
    strategy->setCancelStrokeId(stroke);
    it = queue.insert(it, stroke);
    Q_FOREACH (KisStrokeJobData *jobData, jobsData) {
        stroke->addJob(jobData);
    }
    stroke->endStroke();

    return it;
}

void KisStrokesQueue::Private::startLod0ToNStroke(int levelOfDetail, bool forgettable)
{
    // precondition: lock held!
    // precondition: lod > 0
    KIS_ASSERT_RECOVER_RETURN(levelOfDetail);

    if (!this->lod0ToNStrokeStrategyFactory) return;

    KisLodSyncPair syncPair = this->lod0ToNStrokeStrategyFactory(forgettable);
    executeStrokePair(syncPair, this->strokesQueue, this->strokesQueue.end(),  KisStroke::LODN, levelOfDetail);

    this->lodNNeedsSynchronization = false;
}

void KisStrokesQueue::Private::cancelForgettableStrokes()
{
    if (!strokesQueue.isEmpty() && !hasUnfinishedStrokes()) {
        Q_FOREACH (KisStrokeSP stroke, strokesQueue) {
            KIS_ASSERT_RECOVER_NOOP(stroke->isEnded());

            if (stroke->canForgetAboutMe()) {
                stroke->cancelStroke();
            }
        }
    }
}

bool KisStrokesQueue::Private::canUseLodN() const
{
    Q_FOREACH (KisStrokeSP stroke, strokesQueue) {
        if (stroke->type() == KisStroke::LEGACY) {
            return false;
        }
    }

    return true;
}

bool KisStrokesQueue::Private::shouldWrapInSuspendUpdatesStroke() const
{
    Q_FOREACH (KisStrokeSP stroke, strokesQueue) {
        if (stroke->isCancelled()) continue;

        if (stroke->type() == KisStroke::RESUME) {

            /**
             * Resuming process is long-running and consists of
             * multiple actions, therefore, if it has already started,
             * we cannot use it to guard our new stroke, so just skip it.
             * see https://phabricator.kde.org/T2542
             */
            if (stroke->isInitialized()) continue;

            return false;
        }
    }

    return true;
}

StrokesQueueIterator KisStrokesQueue::Private::findNewLod0Pos()
{
    StrokesQueueIterator it = strokesQueue.begin();
    StrokesQueueIterator end = strokesQueue.end();

    for (; it != end; ++it) {
        if ((*it)->isCancelled()) continue;

        if ((*it)->type() == KisStroke::RESUME) {
            // \see a comment in shouldWrapInSuspendUpdatesStroke()
            if ((*it)->isInitialized()) continue;

            return it;
        }
    }

    return it;
}

StrokesQueueIterator KisStrokesQueue::Private::findNewLodNPos(KisStrokeSP lodN)
{
    StrokesQueueIterator it = strokesQueue.begin();
    StrokesQueueIterator end = strokesQueue.end();

    for (; it != end; ++it) {
        if ((*it)->isCancelled()) continue;

        if (((*it)->type() == KisStroke::SUSPEND ||
             (*it)->type() == KisStroke::RESUME) &&
            (*it)->isInitialized()) {

            // \see a comment in shouldWrapInSuspendUpdatesStroke()
            continue;
        }

        if ((*it)->type() == KisStroke::LOD0 ||
            (*it)->type() == KisStroke::SUSPEND ||
            (*it)->type() == KisStroke::RESUME) {

            if (it != end && it == strokesQueue.begin()) {
                KisStrokeSP head = *it;

                if (head->supportsSuspension()) {
                    head->suspendStroke(lodN);
                }
            }

            return it;
        }
    }

    return it;
}

KisStrokeId KisStrokesQueue::startStroke(KisStrokeStrategy *strokeStrategy)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke;
    KisStrokeStrategy* lodBuddyStrategy;

    m_d->cancelForgettableStrokes();

    if (m_d->desiredLevelOfDetail &&
        m_d->canUseLodN() &&
        (lodBuddyStrategy =
         strokeStrategy->createLodClone(m_d->desiredLevelOfDetail))) {

        if (m_d->lodNNeedsSynchronization) {
            m_d->startLod0ToNStroke(m_d->desiredLevelOfDetail, false);
        }

        stroke = KisStrokeSP(new KisStroke(strokeStrategy, KisStroke::LOD0, 0));

        KisStrokeSP buddy(new KisStroke(lodBuddyStrategy, KisStroke::LODN, m_d->desiredLevelOfDetail));
        lodBuddyStrategy->setCancelStrokeId(buddy);
        stroke->setLodBuddy(buddy);
        m_d->strokesQueue.insert(m_d->findNewLodNPos(buddy), buddy);

        if (m_d->shouldWrapInSuspendUpdatesStroke()) {

            KisSuspendResumePair suspendPair = m_d->suspendUpdatesStrokeStrategyFactory();
            KisSuspendResumePair resumePair = m_d->resumeUpdatesStrokeStrategyFactory();

            StrokesQueueIterator it = m_d->findNewLod0Pos();

            it = executeStrokePair(resumePair, m_d->strokesQueue, it, KisStroke::RESUME, 0);
            it = m_d->strokesQueue.insert(it, stroke);
            it = executeStrokePair(suspendPair, m_d->strokesQueue, it, KisStroke::SUSPEND, 0);

        } else {
            m_d->strokesQueue.insert(m_d->findNewLod0Pos(), stroke);
        }

    } else {
        stroke = KisStrokeSP(new KisStroke(strokeStrategy, KisStroke::LEGACY, 0));
        m_d->strokesQueue.enqueue(stroke);
    }

    KisStrokeId id(stroke);
    strokeStrategy->setCancelStrokeId(id);

    m_d->openedStrokesCounter++;

    if (stroke->type() == KisStroke::LEGACY) {
        m_d->lodNNeedsSynchronization = true;
    }

    return id;
}

void KisStrokesQueue::addJob(KisStrokeId id, KisStrokeJobData *data)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke = id.toStrongRef();
    Q_ASSERT(stroke);

    KisStrokeSP buddy = stroke->lodBuddy();
    if (buddy) {
        KisStrokeJobData *clonedData =
            data->createLodClone(buddy->worksOnLevelOfDetail());
        KIS_ASSERT_RECOVER_RETURN(clonedData);

        buddy->addJob(clonedData);
    }

    stroke->addJob(data);
}

void KisStrokesQueue::endStroke(KisStrokeId id)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke = id.toStrongRef();
    Q_ASSERT(stroke);
    stroke->endStroke();
    m_d->openedStrokesCounter--;

    KisStrokeSP buddy = stroke->lodBuddy();
    if (buddy) {
        buddy->endStroke();
    }
}

bool KisStrokesQueue::cancelStroke(KisStrokeId id)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke = id.toStrongRef();
    if(stroke) {
        stroke->cancelStroke();
        m_d->openedStrokesCounter--;

        KisStrokeSP buddy = stroke->lodBuddy();
        if (buddy) {
            buddy->cancelStroke();
        }
    }
    return stroke;
}

bool KisStrokesQueue::Private::hasUnfinishedStrokes() const
{
    Q_FOREACH (KisStrokeSP stroke, strokesQueue) {
        if (!stroke->isEnded()) {
            return true;
        }
    }

    return false;
}

bool KisStrokesQueue::tryCancelCurrentStrokeAsync()
{
    bool anythingCanceled = false;

    QMutexLocker locker(&m_d->mutex);

    /**
     * We cancel only ended strokes. This is done to avoid
     * handling dangling pointers problem (KisStrokeId). The owner
     * of a stroke will cancel the stroke itself if needed.
     */
    if (!m_d->strokesQueue.isEmpty() &&
        !m_d->hasUnfinishedStrokes()) {

        anythingCanceled = true;

        Q_FOREACH (KisStrokeSP currentStroke, m_d->strokesQueue) {
            KIS_ASSERT_RECOVER_NOOP(currentStroke->isEnded());

            currentStroke->cancelStroke();

            // we shouldn't cancel buddies...
            if (currentStroke->type() == KisStroke::LOD0) {
                /**
                 * If the buddy has already finished, we cannot undo it because
                 * it doesn't store any undo data. Therefore we just regenerate
                 * the LOD caches.
                 */
                m_d->lodNNeedsSynchronization = true;
            }

        }
    }

    /**
     * NOTE: We do not touch the openedStrokesCounter here since
     *       we work with closed id's only here
     */

    return anythingCanceled;
}

void KisStrokesQueue::processQueue(KisUpdaterContext &updaterContext,
                                   bool externalJobsPending)
{
    updaterContext.lock();
    m_d->mutex.lock();

    while(updaterContext.hasSpareThread() &&
          processOneJob(updaterContext,
                        externalJobsPending));

    m_d->mutex.unlock();
    updaterContext.unlock();
}

bool KisStrokesQueue::needsExclusiveAccess() const
{
    return m_d->needsExclusiveAccess;
}

bool KisStrokesQueue::wrapAroundModeSupported() const
{
    return m_d->wrapAroundModeSupported;
}

bool KisStrokesQueue::isEmpty() const
{
    QMutexLocker locker(&m_d->mutex);
    return m_d->strokesQueue.isEmpty();
}

qint32 KisStrokesQueue::sizeMetric() const
{
    QMutexLocker locker(&m_d->mutex);
    if(m_d->strokesQueue.isEmpty()) return 0;

    // just a rough approximation
    return qMax(1, m_d->strokesQueue.head()->numJobs()) * m_d->strokesQueue.size();
}

void KisStrokesQueue::Private::switchDesiredLevelOfDetail(bool forced)
{
    if (forced || nextDesiredLevelOfDetail != desiredLevelOfDetail) {
        Q_FOREACH (KisStrokeSP stroke, strokesQueue) {
            if (stroke->type() != KisStroke::LEGACY)
                return;
        }

        const bool forgettable =
            forced && !lodNNeedsSynchronization &&
            desiredLevelOfDetail == nextDesiredLevelOfDetail;

        desiredLevelOfDetail = nextDesiredLevelOfDetail;
        lodNNeedsSynchronization |= !forgettable;

        if (desiredLevelOfDetail) {
            startLod0ToNStroke(desiredLevelOfDetail, forgettable);
        }
    }
}

void KisStrokesQueue::explicitRegenerateLevelOfDetail()
{
    QMutexLocker locker(&m_d->mutex);
    m_d->switchDesiredLevelOfDetail(true);
}

void KisStrokesQueue::setDesiredLevelOfDetail(int lod)
{
    QMutexLocker locker(&m_d->mutex);

    if (lod == m_d->nextDesiredLevelOfDetail) return;

    m_d->nextDesiredLevelOfDetail = lod;
    m_d->switchDesiredLevelOfDetail(false);
}

void KisStrokesQueue::notifyUFOChangedImage()
{
    QMutexLocker locker(&m_d->mutex);

    m_d->lodNNeedsSynchronization = true;
}

void KisStrokesQueue::setLod0ToNStrokeStrategyFactory(const KisLodSyncStrokeStrategyFactory &factory)
{
    m_d->lod0ToNStrokeStrategyFactory = factory;
}

void KisStrokesQueue::setSuspendUpdatesStrokeStrategyFactory(const KisSuspendResumeStrategyFactory &factory)
{
    m_d->suspendUpdatesStrokeStrategyFactory = factory;
}

void KisStrokesQueue::setResumeUpdatesStrokeStrategyFactory(const KisSuspendResumeStrategyFactory &factory)
{
    m_d->resumeUpdatesStrokeStrategyFactory = factory;
}

KUndo2MagicString KisStrokesQueue::currentStrokeName() const
{
    QMutexLocker locker(&m_d->mutex);
    if(m_d->strokesQueue.isEmpty()) return KUndo2MagicString();

    return m_d->strokesQueue.head()->name();
}

bool KisStrokesQueue::hasOpenedStrokes() const
{
    QMutexLocker locker(&m_d->mutex);
    return m_d->openedStrokesCounter;
}

bool KisStrokesQueue::processOneJob(KisUpdaterContext &updaterContext,
                                    bool externalJobsPending)
{
    if(m_d->strokesQueue.isEmpty()) return false;
    bool result = false;

    qint32 numMergeJobs;
    qint32 numStrokeJobs;
    updaterContext.getJobsSnapshot(numMergeJobs, numStrokeJobs);

    int levelOfDetail = updaterContext.currentLevelOfDetail();

    if(checkStrokeState(numStrokeJobs, levelOfDetail) &&
       checkExclusiveProperty(numMergeJobs, numStrokeJobs) &&
       checkSequentialProperty(numMergeJobs, numStrokeJobs) &&
       checkBarrierProperty(numMergeJobs, numStrokeJobs,
                            externalJobsPending)) {

        KisStrokeSP stroke = m_d->strokesQueue.head();
        updaterContext.addStrokeJob(stroke->popOneJob());
        result = true;
    }

    return result;
}

bool KisStrokesQueue::checkStrokeState(bool hasStrokeJobsRunning,
                                       int runningLevelOfDetail)
{
    KisStrokeSP stroke = m_d->strokesQueue.head();
    bool result = false;

    /**
     * We cannot start/continue a stroke if its LOD differs from
     * the one that is running on CPU
     */
    bool hasLodCompatibility = checkLevelOfDetailProperty(runningLevelOfDetail);
    bool hasJobs = stroke->hasJobs();

    /**
     * The stroke may be cancelled very fast. In this case it will
     * end up in the state:
     *
     * !stroke->isInitialized() && stroke->isEnded() && !stroke->hasJobs()
     *
     * This means that !isInitialised() doesn't imply there are any
     * jobs present.
     */
    if(!stroke->isInitialized() && hasJobs && hasLodCompatibility) {
        /**
         * It might happen that the stroke got initialized, but its job was not
         * started due to some other reasons like exclusivity. Therefore the
         * stroke might end up in loaded, but uninitialized state.
         */
        if (!m_d->currentStrokeLoaded) {
            m_d->needsExclusiveAccess = stroke->isExclusive();
            m_d->wrapAroundModeSupported = stroke->supportsWrapAroundMode();
            m_d->currentStrokeLoaded = true;
        }

        result = true;
    }
    else if(hasJobs && hasLodCompatibility) {
        /**
         * If the stroke has no initialization phase, then it can
         * arrive here unloaded.
         */
        if (!m_d->currentStrokeLoaded) {
            m_d->needsExclusiveAccess = stroke->isExclusive();
            m_d->wrapAroundModeSupported = stroke->supportsWrapAroundMode();
            m_d->currentStrokeLoaded = true;
        }

        result = true;
    }
    else if(stroke->isEnded() && !hasJobs && !hasStrokeJobsRunning) {
        m_d->strokesQueue.dequeue(); // deleted by shared pointer
        m_d->needsExclusiveAccess = false;
        m_d->wrapAroundModeSupported = false;
        m_d->currentStrokeLoaded = false;

        m_d->switchDesiredLevelOfDetail(false);

        if(!m_d->strokesQueue.isEmpty()) {
            result = checkStrokeState(false, runningLevelOfDetail);
        }
    }

    return result;
}

bool KisStrokesQueue::checkExclusiveProperty(qint32 numMergeJobs,
                                             qint32 numStrokeJobs)
{
    if(!m_d->strokesQueue.head()->isExclusive()) return true;
    Q_UNUSED(numMergeJobs);
    Q_UNUSED(numStrokeJobs);
    Q_ASSERT(!(numMergeJobs && numStrokeJobs));
    return numMergeJobs == 0;
}

bool KisStrokesQueue::checkSequentialProperty(qint32 numMergeJobs,
                                              qint32 numStrokeJobs)
{
    Q_UNUSED(numMergeJobs);

    KisStrokeSP stroke = m_d->strokesQueue.head();
    if(!stroke->prevJobSequential() && !stroke->nextJobSequential()) return true;

    Q_ASSERT(!stroke->prevJobSequential() || numStrokeJobs <= 1);
    return numStrokeJobs == 0;
}

bool KisStrokesQueue::checkBarrierProperty(qint32 numMergeJobs,
                                           qint32 numStrokeJobs,
                                           bool externalJobsPending)
{
    KisStrokeSP stroke = m_d->strokesQueue.head();
    if(!stroke->nextJobBarrier()) return true;

    return !numMergeJobs && !numStrokeJobs && !externalJobsPending;
}

bool KisStrokesQueue::checkLevelOfDetailProperty(int runningLevelOfDetail)
{
    KisStrokeSP stroke = m_d->strokesQueue.head();

    return runningLevelOfDetail < 0 ||
        stroke->worksOnLevelOfDetail() == runningLevelOfDetail;
}
