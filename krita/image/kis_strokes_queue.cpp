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

typedef QQueue<KisStrokeSP> StrokesQueue;
typedef QQueue<KisStrokeSP>::iterator StrokesQueueIterator;

struct KisStrokesQueue::Private {
    Private()
        : openedStrokesCounter(0),
          needsExclusiveAccess(false),
          wrapAroundModeSupported(false),
          lodNNeedsSynchronization(true),
          desiredLevelOfDetail(0),
          nextDesiredLevelOfDetail(0) {}

    StrokesQueue strokesQueue;
    int openedStrokesCounter;
    bool needsExclusiveAccess;
    bool wrapAroundModeSupported;
    bool lodNNeedsSynchronization;
    int desiredLevelOfDetail;
    int nextDesiredLevelOfDetail;
    QMutex mutex;
    KisStrokeStrategyFactory lod0ToNStrokeStrategyFactory;
    KisStrokeStrategyFactory suspendUpdatesStrokeStrategyFactory;
    KisStrokeStrategyFactory resumeUpdatesStrokeStrategyFactory;

    void startLod0ToNStroke(int levelOfDetail);

    bool canUseLodN() const;
    StrokesQueueIterator findNewLod0Pos();
    StrokesQueueIterator findNewLodNPos(KisStrokeSP lodN);
    bool shouldWrapInSuspendUpdatesStroke() const;

    void switchDesiredLevelOfDetail();
    bool hasUnfinishedStrokes() const;
};


KisStrokesQueue::KisStrokesQueue()
  : m_d(new Private)
{
}

KisStrokesQueue::~KisStrokesQueue()
{
    foreach(KisStrokeSP stroke, m_d->strokesQueue) {
        stroke->cancelStroke();
    }

    delete m_d;
}

void KisStrokesQueue::Private::startLod0ToNStroke(int levelOfDetail)
{
    // precondition: lock held!
    // precondition: lod > 0
    KIS_ASSERT_RECOVER_RETURN(levelOfDetail);

    if (!this->lod0ToNStrokeStrategyFactory) return;

    KisStrokeStrategy *lod0ToN = this->lod0ToNStrokeStrategyFactory();
    KisStrokeSP sync(new KisStroke(lod0ToN, KisStroke::LODN, levelOfDetail));
    lod0ToN->setCancelStrokeId(sync);
    this->strokesQueue.enqueue(sync);
    sync->endStroke();
    this->lodNNeedsSynchronization = false;
}

bool KisStrokesQueue::Private::canUseLodN() const
{
    foreach (KisStrokeSP stroke, strokesQueue) {
        if (stroke->type() == KisStroke::LEGACY) {
            return false;
        }
    }

    return true;
}

bool KisStrokesQueue::Private::shouldWrapInSuspendUpdatesStroke() const
{
    foreach (KisStrokeSP stroke, strokesQueue) {
        if (stroke->type() == KisStroke::RESUME) {
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
        if ((*it)->type() == KisStroke::RESUME) {
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

    if (m_d->desiredLevelOfDetail &&
        m_d->canUseLodN() &&
        (lodBuddyStrategy =
         strokeStrategy->createLodClone(m_d->desiredLevelOfDetail))) {

        if (m_d->lodNNeedsSynchronization) {
            m_d->startLod0ToNStroke(m_d->desiredLevelOfDetail);
        }

        stroke = KisStrokeSP(new KisStroke(strokeStrategy, KisStroke::LOD0, 0));

        KisStrokeSP buddy(new KisStroke(lodBuddyStrategy, KisStroke::LODN, m_d->desiredLevelOfDetail));
        lodBuddyStrategy->setCancelStrokeId(buddy);
        stroke->setLodBuddy(buddy);
        m_d->strokesQueue.insert(m_d->findNewLodNPos(buddy), buddy);

        if (m_d->shouldWrapInSuspendUpdatesStroke()) {
            KisStrokeStrategy *strategy;

            strategy = m_d->suspendUpdatesStrokeStrategyFactory();
            KisStrokeSP suspend(new KisStroke(strategy, KisStroke::SUSPEND, 0));
            strategy->setCancelStrokeId(suspend);

            strategy = m_d->resumeUpdatesStrokeStrategyFactory();
            KisStrokeSP resume(new KisStroke(strategy, KisStroke::RESUME, 0));
            strategy->setCancelStrokeId(resume);

            StrokesQueueIterator it = m_d->findNewLod0Pos();

            it = m_d->strokesQueue.insert(it, resume);
            resume->endStroke();

            it = m_d->strokesQueue.insert(it, stroke);

            it = m_d->strokesQueue.insert(it, suspend);
            suspend->endStroke();

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
    stroke->addJob(data);

    KisStrokeSP buddy = stroke->lodBuddy();
    if (buddy) {
        KisStrokeJobData *clonedData =
            data->createLodClone(buddy->worksOnLevelOfDetail());
        KIS_ASSERT_RECOVER_RETURN(clonedData);

        buddy->addJob(clonedData);
    }
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
    foreach (KisStrokeSP stroke, strokesQueue) {
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

        foreach (KisStrokeSP currentStroke, m_d->strokesQueue) {
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
    return m_d->strokesQueue.head()->numJobs() * m_d->strokesQueue.size();
}

void KisStrokesQueue::Private::switchDesiredLevelOfDetail()
{
    if (nextDesiredLevelOfDetail != desiredLevelOfDetail) {
        foreach (KisStrokeSP stroke, strokesQueue) {
            if (stroke->type() != KisStroke::LEGACY)
                return;
        }

        desiredLevelOfDetail = nextDesiredLevelOfDetail;
        lodNNeedsSynchronization = true;

        if (desiredLevelOfDetail) {
            startLod0ToNStroke(desiredLevelOfDetail);
        }
    }
}

void KisStrokesQueue::setDesiredLevelOfDetail(int lod)
{
    QMutexLocker locker(&m_d->mutex);

    if (lod == m_d->nextDesiredLevelOfDetail) return;

    m_d->nextDesiredLevelOfDetail = lod;
    m_d->switchDesiredLevelOfDetail();
}

void KisStrokesQueue::notifyUFOChangedImage()
{
    QMutexLocker locker(&m_d->mutex);

    m_d->lodNNeedsSynchronization = true;
}

void KisStrokesQueue::setLod0ToNStrokeStrategyFactory(const KisStrokeStrategyFactory &factory)
{
    m_d->lod0ToNStrokeStrategyFactory = factory;
}

void KisStrokesQueue::setSuspendUpdatesStrokeStrategyFactory(const KisStrokeStrategyFactory &factory)
{
    m_d->suspendUpdatesStrokeStrategyFactory = factory;
}

void KisStrokesQueue::setResumeUpdatesStrokeStrategyFactory(const KisStrokeStrategyFactory &factory)
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
        m_d->needsExclusiveAccess = stroke->isExclusive();
        m_d->wrapAroundModeSupported = stroke->supportsWrapAroundMode();

        result = true;
    }
    else if(hasJobs && hasLodCompatibility) {
        result = true;
    }
    else if(stroke->isEnded() && !hasJobs && !hasStrokeJobsRunning) {
        m_d->strokesQueue.dequeue(); // deleted by shared pointer
        m_d->needsExclusiveAccess = false;
        m_d->wrapAroundModeSupported = false;

        m_d->switchDesiredLevelOfDetail();

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
