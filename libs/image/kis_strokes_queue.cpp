/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_strokes_queue.h"

#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include "kis_stroke.h"
#include "kis_updater_context.h"
#include "kis_stroke_job_strategy.h"
#include "kis_stroke_strategy.h"
#include "kis_undo_stores.h"
#include "kis_post_execution_undo_adapter.h"
#include "KisCppQuirks.h"

typedef QQueue<KisStrokeSP> StrokesQueue;
typedef QQueue<KisStrokeSP>::iterator StrokesQueueIterator;

#include "kis_image_interfaces.h"
class KisStrokesQueue::LodNUndoStrokesFacade : public KisStrokesFacade
{
public:
    LodNUndoStrokesFacade(KisStrokesQueue *_q) : q(_q) {}

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy) override {
        return q->startLodNUndoStroke(strokeStrategy);
    }

    void addJob(KisStrokeId id, KisStrokeJobData *data) override {
        KisStrokeSP stroke = id.toStrongRef();
        KIS_SAFE_ASSERT_RECOVER_NOOP(stroke);
        KIS_SAFE_ASSERT_RECOVER_NOOP(!stroke->lodBuddy());
        KIS_SAFE_ASSERT_RECOVER_NOOP(stroke->type() == KisStroke::LODN);

        q->addJob(id, data);
    }

    void endStroke(KisStrokeId id) override {
        KisStrokeSP stroke = id.toStrongRef();
        KIS_SAFE_ASSERT_RECOVER_NOOP(stroke);
        KIS_SAFE_ASSERT_RECOVER_NOOP(!stroke->lodBuddy());
        KIS_SAFE_ASSERT_RECOVER_NOOP(stroke->type() == KisStroke::LODN);

        q->endStroke(id);
    }

    bool cancelStroke(KisStrokeId id) override {
        Q_UNUSED(id);
        qFatal("Not implemented");
        return false;
    }

private:
    KisStrokesQueue *q;
};


struct Q_DECL_HIDDEN KisStrokesQueue::Private {
    Private(KisStrokesQueue *_q)
        : q(_q),
          openedStrokesCounter(0),
          needsExclusiveAccess(false),
          wrapAroundModeSupported(false),
          balancingRatioOverride(-1.0),
          currentStrokeLoaded(false),
          lodNNeedsSynchronization(true),
          desiredLevelOfDetail(0),
          nextDesiredLevelOfDetail(0),
          lodNStrokesFacade(_q),
          lodNPostExecutionUndoAdapter(&lodNUndoStore, &lodNStrokesFacade) {}

    KisStrokesQueue *q;
    StrokesQueue strokesQueue;
    int openedStrokesCounter;
    bool needsExclusiveAccess;
    bool wrapAroundModeSupported;
    qreal balancingRatioOverride;
    bool currentStrokeLoaded;

    bool lodNNeedsSynchronization;
    int desiredLevelOfDetail;
    int nextDesiredLevelOfDetail;
    QMutex mutex;
    KisLodSyncStrokeStrategyFactory lod0ToNStrokeStrategyFactory;
    KisSuspendResumeStrategyPairFactory suspendResumeUpdatesStrokeStrategyFactory;
    KisSurrogateUndoStore lodNUndoStore;
    LodNUndoStrokesFacade lodNStrokesFacade;
    KisPostExecutionUndoAdapter lodNPostExecutionUndoAdapter;

    void cancelForgettableStrokes();
    void startLod0ToNStroke(int levelOfDetail, bool forgettable);


    std::pair<StrokesQueueIterator, StrokesQueueIterator> currentLodRange();
    StrokesQueueIterator findNewLod0Pos();
    StrokesQueueIterator findNewLodNPos(KisStrokeSP lodN);
    bool shouldWrapInSuspendUpdatesStroke();

    void switchDesiredLevelOfDetail(bool forced);
    bool hasUnfinishedStrokes() const;
    void tryClearUndoOnStrokeCompletion(KisStrokeSP finishingStroke);
};


KisStrokesQueue::KisStrokesQueue()
  : m_d(new Private(this))
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
executeStrokePair(const StrokePair &pair, StrokesQueue &queue, typename StrokesQueue::iterator it, KisStroke::Type type, int levelOfDetail, KisStrokesQueueMutatedJobInterface *mutatedJobsInterface) {
    KisStrokeStrategy *strategy = pair.first;
    QList<KisStrokeJobData*> jobsData = pair.second;

    KisStrokeSP stroke(new KisStroke(strategy, type, levelOfDetail));
    strategy->setMutatedJobsInterface(mutatedJobsInterface, stroke);
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

    {
        // sanity check: there should be no open LoD range now!
        StrokesQueueIterator it;
        StrokesQueueIterator end;
        std::tie(it, end) = currentLodRange();
        KIS_SAFE_ASSERT_RECOVER_NOOP(it == end);
    }

    if (!this->lod0ToNStrokeStrategyFactory) return;

    KisLodSyncPair syncPair = this->lod0ToNStrokeStrategyFactory(forgettable);
    executeStrokePair(syncPair, this->strokesQueue, this->strokesQueue.end(),  KisStroke::LODN, levelOfDetail, q);

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

std::pair<StrokesQueueIterator, StrokesQueueIterator> KisStrokesQueue::Private::currentLodRange()
{
    /**
     * LoD-capable strokes should live in the range after the last
     * legacy stroke of the queue
     */

    for (auto it = std::make_reverse_iterator(strokesQueue.end());
         it != std::make_reverse_iterator(strokesQueue.begin());
         ++it) {

        if ((*it)->type() == KisStroke::LEGACY) {
            // it.base() returns the next element after the one we found
            return std::make_pair(it.base(), strokesQueue.end());
        }
    }

    return std::make_pair(strokesQueue.begin(), strokesQueue.end());
}

bool KisStrokesQueue::Private::shouldWrapInSuspendUpdatesStroke()
{
    StrokesQueueIterator it;
    StrokesQueueIterator end;
    std::tie(it, end) = currentLodRange();

    for (; it != end; ++it) {
        KisStrokeSP stroke = *it;

        if (stroke->isCancelled()) continue;

        if (stroke->type() == KisStroke::RESUME) {
            return false;
        }
    }

    return true;
}

StrokesQueueIterator KisStrokesQueue::Private::findNewLod0Pos()
{
    StrokesQueueIterator it;
    StrokesQueueIterator end;
    std::tie(it, end) = currentLodRange();

    for (; it != end; ++it) {
        if ((*it)->isCancelled()) continue;

        if ((*it)->type() == KisStroke::RESUME) {
            return it;
        }
    }

    return it;
}

StrokesQueueIterator KisStrokesQueue::Private::findNewLodNPos(KisStrokeSP lodN)
{
    StrokesQueueIterator it;
    StrokesQueueIterator end;
    std::tie(it, end) = currentLodRange();

    for (; it != end; ++it) {
        if ((*it)->isCancelled()) continue;

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

KisStrokeId KisStrokesQueue::startLodNUndoStroke(KisStrokeStrategy *strokeStrategy)
{
    QMutexLocker locker(&m_d->mutex);

    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->lodNNeedsSynchronization);
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->desiredLevelOfDetail > 0);

    KisStrokeSP buddy(new KisStroke(strokeStrategy, KisStroke::LODN, m_d->desiredLevelOfDetail));
    strokeStrategy->setMutatedJobsInterface(this, buddy);
    m_d->strokesQueue.insert(m_d->findNewLodNPos(buddy), buddy);

    KisStrokeId id(buddy);
    m_d->openedStrokesCounter++;

    return id;
}

KisStrokeId KisStrokesQueue::startStroke(KisStrokeStrategy *strokeStrategy)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke;
    KisStrokeStrategy* lodBuddyStrategy;

    // we should let forgettable strokes to queue up
    if (!strokeStrategy->canForgetAboutMe()) {
        m_d->cancelForgettableStrokes();
    }

    if (m_d->desiredLevelOfDetail &&
        (lodBuddyStrategy =
         strokeStrategy->createLodClone(m_d->desiredLevelOfDetail))) {

        KisStrokeStrategy *legacyInitializingStrategy = strokeStrategy->createLegacyInitializingStroke();
        if (legacyInitializingStrategy) {
            // this strategy must be legacy, that is without lod support
            KIS_SAFE_ASSERT_RECOVER_NOOP(!legacyInitializingStrategy->createLodClone(m_d->desiredLevelOfDetail));

            KisStrokeSP legacyInitializingStroke(new KisStroke(legacyInitializingStrategy, KisStroke::LEGACY, 0));
            legacyInitializingStrategy->setMutatedJobsInterface(this, legacyInitializingStroke);
            m_d->strokesQueue.enqueue(legacyInitializingStroke);
            legacyInitializingStroke->endStroke();
            m_d->lodNNeedsSynchronization = true;
        }

        if (m_d->lodNNeedsSynchronization) {
            m_d->startLod0ToNStroke(m_d->desiredLevelOfDetail, false);
        }

        stroke = KisStrokeSP(new KisStroke(strokeStrategy, KisStroke::LOD0, 0));

        KisStrokeSP buddy(new KisStroke(lodBuddyStrategy, KisStroke::LODN, m_d->desiredLevelOfDetail));
        lodBuddyStrategy->setMutatedJobsInterface(this, buddy);
        stroke->setLodBuddy(buddy);
        m_d->strokesQueue.insert(m_d->findNewLodNPos(buddy), buddy);

        if (m_d->shouldWrapInSuspendUpdatesStroke()) {

            KisSuspendResumePair suspendPair;
            KisSuspendResumePair resumePair;
            std::tie(suspendPair, resumePair) = m_d->suspendResumeUpdatesStrokeStrategyFactory();

            StrokesQueueIterator it = m_d->findNewLod0Pos();

            it = executeStrokePair(resumePair, m_d->strokesQueue, it, KisStroke::RESUME, 0, this);
            it = m_d->strokesQueue.insert(it, stroke);
            it = executeStrokePair(suspendPair, m_d->strokesQueue, it, KisStroke::SUSPEND, 0, this);

        } else {
            m_d->strokesQueue.insert(m_d->findNewLod0Pos(), stroke);
        }

    } else {

        KisStrokeStrategy *legacyInitializingStrategy = strokeStrategy->createLegacyInitializingStroke();
        if (legacyInitializingStrategy) {
            // this strategy must be legacy, that is without lod support
            KIS_SAFE_ASSERT_RECOVER_NOOP(!legacyInitializingStrategy->createLodClone(m_d->desiredLevelOfDetail));

            KisStrokeSP legacyInitializingStroke(new KisStroke(legacyInitializingStrategy, KisStroke::LEGACY, 0));
            legacyInitializingStrategy->setMutatedJobsInterface(this, legacyInitializingStroke);
            m_d->strokesQueue.enqueue(legacyInitializingStroke);
            legacyInitializingStroke->endStroke();
            m_d->lodNNeedsSynchronization = true;
        }


        stroke = KisStrokeSP(new KisStroke(strokeStrategy, KisStroke::LEGACY, 0));
        m_d->strokesQueue.enqueue(stroke);
    }

    KisStrokeId id(stroke);
    strokeStrategy->setMutatedJobsInterface(this, id);

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
    KIS_SAFE_ASSERT_RECOVER_RETURN(stroke);

    KisStrokeSP buddy = stroke->lodBuddy();
    if (buddy) {
        KisStrokeJobData *clonedData =
            data->createLodClone(buddy->worksOnLevelOfDetail());
        KIS_ASSERT_RECOVER_RETURN(clonedData);

        buddy->addJob(clonedData);
    }

    stroke->addJob(data);
}

void KisStrokesQueue::addMutatedJobs(KisStrokeId id, const QVector<KisStrokeJobData *> list)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke = id.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(stroke);

    stroke->addMutatedJobs(list);
}

void KisStrokesQueue::endStroke(KisStrokeId id)
{
    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP stroke = id.toStrongRef();
    KIS_SAFE_ASSERT_RECOVER_RETURN(stroke);
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

UndoResult KisStrokesQueue::tryUndoLastStrokeAsync()
{
    UndoResult result = UNDO_FAIL;

    QMutexLocker locker(&m_d->mutex);

    KisStrokeSP lastStroke;
    KisStrokeSP lastBuddy;
    bool buddyFound = false;

    for (auto it = std::make_reverse_iterator(m_d->strokesQueue.constEnd());
         it != std::make_reverse_iterator(m_d->strokesQueue.constBegin());
         ++it) {

        if ((*it)->type() == KisStroke::LEGACY) {
            break;
        }

        if (!lastStroke && (*it)->type() == KisStroke::LOD0 && !(*it)->isCancelled()) {
            lastStroke = *it;
            lastBuddy = lastStroke->lodBuddy();

            KIS_SAFE_ASSERT_RECOVER(lastBuddy) {
                lastStroke.clear();
                lastBuddy.clear();
                break;
            }
        }

        KIS_SAFE_ASSERT_RECOVER(!lastStroke || *it == lastBuddy || (*it)->type() != KisStroke::LODN) {
            lastStroke.clear();
            lastBuddy.clear();
            break;
        }

        if (lastStroke && *it == lastBuddy) {
            KIS_SAFE_ASSERT_RECOVER(lastBuddy->type() == KisStroke::LODN) {
                lastStroke.clear();
                lastBuddy.clear();
                break;
            }
            buddyFound = true;
            break;
        }
    }

    if (!lastStroke) return UNDO_FAIL;
    if (!lastStroke->isEnded()) return UNDO_FAIL;
    if (lastStroke->isCancelled()) return UNDO_FAIL;

    KIS_SAFE_ASSERT_RECOVER_NOOP(!buddyFound ||
                                 lastStroke->isCancelled() == lastBuddy->isCancelled());
    KIS_SAFE_ASSERT_RECOVER_NOOP(lastBuddy->isEnded());

    if (!lastStroke->canCancel()) {
        return UNDO_WAIT;
    }
    lastStroke->cancelStroke();

    if (buddyFound && lastBuddy->canCancel()) {
        lastBuddy->cancelStroke();
    } else {
        // TODO: assert that checks that there is no other lodn strokes
        locker.unlock();
        m_d->lodNUndoStore.undo();
        m_d->lodNUndoStore.purgeRedoState();
        locker.relock();
    }

    result = UNDO_OK;

    return result;
}

void KisStrokesQueue::Private::tryClearUndoOnStrokeCompletion(KisStrokeSP finishingStroke)
{
    if (finishingStroke->type() != KisStroke::RESUME) return;

    bool hasResumeStrokes = false;
    bool hasLod0Strokes = false;

    auto it = std::find(strokesQueue.begin(), strokesQueue.end(), finishingStroke);
    KIS_SAFE_ASSERT_RECOVER_RETURN(it != strokesQueue.end());
    ++it;

    for (; it != strokesQueue.end(); ++it) {
        KisStrokeSP stroke = *it;
        if (stroke->type() == KisStroke::LEGACY) break;

        hasLod0Strokes |= stroke->type() == KisStroke::LOD0;
        hasResumeStrokes |= stroke->type() == KisStroke::RESUME;
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(!hasLod0Strokes || hasResumeStrokes);

    if (!hasResumeStrokes && !hasLod0Strokes) {
        lodNUndoStore.clear();
    }
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

qreal KisStrokesQueue::balancingRatioOverride() const
{
    return m_d->balancingRatioOverride;
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

void KisStrokesQueue::debugDumpAllStrokes()
{
    QMutexLocker locker(&m_d->mutex);

    qDebug() <<"===";
    Q_FOREACH (KisStrokeSP stroke, m_d->strokesQueue) {
        qDebug() << ppVar(stroke->name()) << ppVar(stroke->type()) << ppVar(stroke->numJobs()) << ppVar(stroke->isInitialized()) << ppVar(stroke->isCancelled());
    }
    qDebug() <<"===";
}

void KisStrokesQueue::setLod0ToNStrokeStrategyFactory(const KisLodSyncStrokeStrategyFactory &factory)
{
    m_d->lod0ToNStrokeStrategyFactory = factory;
}

void KisStrokesQueue::setSuspendResumeUpdatesStrokeStrategyFactory(const KisSuspendResumeStrategyPairFactory &factory)
{
    m_d->suspendResumeUpdatesStrokeStrategyFactory = factory;
}

KisPostExecutionUndoAdapter *KisStrokesQueue::lodNPostExecutionUndoAdapter() const
{
    return &m_d->lodNPostExecutionUndoAdapter;
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

    const int levelOfDetail = updaterContext.currentLevelOfDetail();

    const KisUpdaterContextSnapshotEx snapshot = updaterContext.getContextSnapshotEx();

    const bool hasStrokeJobs = !(snapshot == ContextEmpty ||
                                 snapshot == HasMergeJob);
    const bool hasMergeJobs = snapshot & HasMergeJob;

    if(checkStrokeState(hasStrokeJobs, levelOfDetail) &&
       checkExclusiveProperty(hasMergeJobs, hasStrokeJobs) &&
       checkSequentialProperty(snapshot, externalJobsPending)) {

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
            m_d->balancingRatioOverride = stroke->balancingRatioOverride();
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
            m_d->balancingRatioOverride = stroke->balancingRatioOverride();
            m_d->currentStrokeLoaded = true;
        }

        result = true;
    }
    else if(stroke->isEnded() && !hasJobs && !hasStrokeJobsRunning) {
        m_d->tryClearUndoOnStrokeCompletion(stroke);

        m_d->strokesQueue.dequeue(); // deleted by shared pointer
        m_d->needsExclusiveAccess = false;
        m_d->wrapAroundModeSupported = false;
        m_d->balancingRatioOverride = -1.0;
        m_d->currentStrokeLoaded = false;

        m_d->switchDesiredLevelOfDetail(false);

        if(!m_d->strokesQueue.isEmpty()) {
            result = checkStrokeState(false, runningLevelOfDetail);
        }
    }

    return result;
}

bool KisStrokesQueue::checkExclusiveProperty(bool hasMergeJobs,
                                             bool hasStrokeJobs)
{
    Q_UNUSED(hasStrokeJobs);

    if(!m_d->strokesQueue.head()->isExclusive()) return true;
    return hasMergeJobs == 0;
}

bool KisStrokesQueue::checkSequentialProperty(KisUpdaterContextSnapshotEx snapshot,
                                              bool externalJobsPending)
{
    KisStrokeSP stroke = m_d->strokesQueue.head();

    if (snapshot & HasSequentialJob ||
        snapshot & HasBarrierJob) {
        return false;
    }

    KisStrokeJobData::Sequentiality nextSequentiality =
        stroke->nextJobSequentiality();

    if (nextSequentiality == KisStrokeJobData::UNIQUELY_CONCURRENT &&
        snapshot & HasUniquelyConcurrentJob) {

        return false;
    }

    if (nextSequentiality == KisStrokeJobData::SEQUENTIAL &&
        (snapshot & HasUniquelyConcurrentJob ||
         snapshot & HasConcurrentJob)) {

        return false;
    }

    if (nextSequentiality == KisStrokeJobData::BARRIER &&
        (snapshot & HasUniquelyConcurrentJob ||
         snapshot & HasConcurrentJob ||
         snapshot & HasMergeJob ||
         externalJobsPending)) {

        return false;
    }

    return true;
}

bool KisStrokesQueue::checkLevelOfDetailProperty(int runningLevelOfDetail)
{
    KisStrokeSP stroke = m_d->strokesQueue.head();

    return runningLevelOfDetail < 0 ||
        stroke->nextJobLevelOfDetail() == runningLevelOfDetail;
}
