/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_suspend_projection_updates_stroke_strategy.h"

#include <kis_image.h>
#include <krita_utils.h>
#include <kis_projection_updates_filter.h>
#include "kis_image_signal_router.h"

#include "kundo2command.h"
#include "KisRunnableStrokeJobDataBase.h"
#include "KisRunnableStrokeJobsInterface.h"
#include "kis_paintop_utils.h"


inline uint qHash(const QRect &rc) {
    return rc.x() +
        (rc.y() << 16) +
        (rc.width() << 8) +
        (rc.height() << 24);
}

struct KisSuspendProjectionUpdatesStrokeStrategy::Private
{
    KisImageWSP image;
    bool suspend;
    QVector<QRect> accumulatedDirtyRects;
    bool sanityResumingFinished = false;
    int updatesEpoch = 0;
    bool haveDisabledGUILodSync = false;

    void tryFetchUsedUpdatesFilter(KisImageSP image);
    void tryIssueRecordedDirtyRequests(KisImageSP image);

    class SuspendLod0Updates : public KisProjectionUpdatesFilter
    {

        struct Request {
            Request() : resetAnimationCache(false) {}
            Request(const QRect &_rect, bool _resetAnimationCache)
                : rect(_rect), resetAnimationCache(_resetAnimationCache)
            {
            }

            QRect rect;
            bool resetAnimationCache;
        };

        typedef QHash<KisNodeSP, QVector<Request> > RectsHash;
    public:
        SuspendLod0Updates()
        {
        }

        bool filter(KisImage *image, KisNode *node, const QVector<QRect> &rects,  bool resetAnimationCache) override {
            if (image->currentLevelOfDetail() > 0) return false;

            QMutexLocker l(&m_mutex);

            Q_FOREACH(const QRect &rc, rects) {
                m_requestsHash[KisNodeSP(node)].append(Request(rc, resetAnimationCache));
            }

            return true;
        }

        static inline QRect alignRect(const QRect &rc, const int step) {
            static const int decstep = step - 1;
            static const int invstep = ~decstep;

            int x0, y0, x1, y1;
            rc.getCoords(&x0, &y0, &x1, &y1);

            x0 &= invstep;
            y0 &= invstep;
            x1 |= decstep;
            y1 |= decstep;

            QRect result;
            result.setCoords(x0, y0, x1, y1);
            return result;
        }

        void notifyUpdates(KisNodeGraphListener *listener) {
            RectsHash::const_iterator it = m_requestsHash.constBegin();
            RectsHash::const_iterator end = m_requestsHash.constEnd();

            const int step = 64;

            for (; it != end; ++it) {
                KisNodeSP node = it.key();

                QRegion region;

                bool resetAnimationCache = false;
                Q_FOREACH (const Request &req, it.value()) {
                    region += alignRect(req.rect, step);
                    resetAnimationCache |= req.resetAnimationCache;
                }

                // FIXME: constness: port rPU to SP
                listener->requestProjectionUpdate(const_cast<KisNode*>(node.data()), region.rects(), resetAnimationCache);
            }
        }

    private:
        RectsHash m_requestsHash;
        QMutex m_mutex;
    };

    QVector<QSharedPointer<SuspendLod0Updates>> usedFilters;


    struct StrokeJobCommand : public KUndo2Command
    {
        StrokeJobCommand(KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                         KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL)
            : m_sequentiality(sequentiality),
              m_exclusivity(exclusivity)
        {}

        KisStrokeJobData::Sequentiality m_sequentiality;
        KisStrokeJobData::Exclusivity m_exclusivity;
    };

    struct UndoableData : public KisRunnableStrokeJobDataBase
    {
        UndoableData(StrokeJobCommand *command)
            : KisRunnableStrokeJobDataBase(command->m_sequentiality, command->m_exclusivity),
              m_command(command)
        {
        }

        void run() override {
            KIS_SAFE_ASSERT_RECOVER_RETURN(m_command);
            m_command->redo();
        }

        QScopedPointer<StrokeJobCommand> m_command;
    };

    // Suspend job should be a barrier to ensure all
    // previous lodN strokes reach the GUI. Otherwise,
    // they will be blocked in
    // KisImage::notifyProjectionUpdated()
    struct SuspendUpdatesCommand : public StrokeJobCommand
    {
        SuspendUpdatesCommand(Private *d)
            : StrokeJobCommand(KisStrokeJobData::BARRIER),
              m_d(d) {}

        void redo() override {
            KisImageSP image = m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);
            KIS_SAFE_ASSERT_RECOVER_RETURN(!image->projectionUpdatesFilter());

            image->setProjectionUpdatesFilter(
                KisProjectionUpdatesFilterSP(new Private::SuspendLod0Updates()));
        }


        void undo() override {
            KisImageSP image = m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);
            KIS_SAFE_ASSERT_RECOVER_RETURN(image->projectionUpdatesFilter());

            m_d->tryFetchUsedUpdatesFilter(image);
        }

        Private *m_d;
    };


    struct ResumeAndIssueGraphUpdatesCommand : public StrokeJobCommand
    {
        ResumeAndIssueGraphUpdatesCommand(Private *d)
            : StrokeJobCommand(KisStrokeJobData::BARRIER),
              m_d(d) {}

        void redo() override {
            KisImageSP image = m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);
            KIS_SAFE_ASSERT_RECOVER_RETURN(image->projectionUpdatesFilter());

            image->disableUIUpdates();
            m_d->tryFetchUsedUpdatesFilter(image);
            m_d->tryIssueRecordedDirtyRequests(image);
        }

        void undo() override {
            KisImageSP image = m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);
            KIS_SAFE_ASSERT_RECOVER_RETURN(!image->projectionUpdatesFilter());

            image->setProjectionUpdatesFilter(
                KisProjectionUpdatesFilterSP(new Private::SuspendLod0Updates()));
            image->enableUIUpdates();
        }

        Private *m_d;
    };

    struct UploadDataToUIData : public KisRunnableStrokeJobDataBase
    {
        UploadDataToUIData(const QRect &rc, int updateEpoch, KisSuspendProjectionUpdatesStrokeStrategy *strategy)
            : KisRunnableStrokeJobDataBase(KisStrokeJobData::CONCURRENT),
              m_strategy(strategy),
              m_rc(rc),
              m_updateEpoch(updateEpoch)
        {
        }

        void run() override {
            // check if we've already started stinking...
            if (m_strategy->m_d->updatesEpoch > m_updateEpoch) {
                return;
            }

            KisImageSP image = m_strategy->m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);

            image->notifyProjectionUpdated(m_rc);
        }

        KisSuspendProjectionUpdatesStrokeStrategy *m_strategy;
        QRect m_rc;
        int m_updateEpoch;
    };

    struct BlockUILodSync : public KisRunnableStrokeJobDataBase
    {
        BlockUILodSync(bool block, KisSuspendProjectionUpdatesStrokeStrategy *strategy)
            : KisRunnableStrokeJobDataBase(KisStrokeJobData::BARRIER),
              m_strategy(strategy),
              m_block(block)
        {}

        void run() override {
            KisImageSP image = m_strategy->m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);

            image->signalRouter()->emitRequestLodPlanesSyncBlocked(m_block);
            m_strategy->m_d->haveDisabledGUILodSync = m_block;
        }

        KisSuspendProjectionUpdatesStrokeStrategy *m_strategy;
        bool m_block;
    };

    struct StartBatchUIUpdatesCommand : public StrokeJobCommand
    {
        StartBatchUIUpdatesCommand(KisSuspendProjectionUpdatesStrokeStrategy *strategy)
            : StrokeJobCommand(KisStrokeJobData::BARRIER),
              m_strategy(strategy) {}

        void redo() override {
            KisImageSP image = m_strategy->m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);

            /**
             * We accumulate dirty rects from all(!) epochs, because some updates of the
             * previous epochs might have been cancelled without doing any real work.
             */
            const QVector<QRect> totalDirtyRects =
                image->enableUIUpdates() + m_strategy->m_d->accumulatedDirtyRects;

            const QRect totalRect =
                image->bounds() &
                std::accumulate(totalDirtyRects.begin(), totalDirtyRects.end(), QRect(), std::bit_or<QRect>());

            m_strategy->m_d->accumulatedDirtyRects =
                KisPaintOpUtils::splitAndFilterDabRect(totalRect,
                                                       totalDirtyRects,
                                                       KritaUtils::optimalPatchSize().width());

            image->signalRouter()->emitNotifyBatchUpdateStarted();

            QVector<KisRunnableStrokeJobDataBase*> jobsData;
            Q_FOREACH (const QRect &rc, m_strategy->m_d->accumulatedDirtyRects) {
                jobsData << new Private::UploadDataToUIData(rc, m_strategy->m_d->updatesEpoch, m_strategy);
            }

            m_strategy->runnableJobsInterface()->addRunnableJobs(jobsData);

        }

        void undo() override {
            KisImageSP image = m_strategy->m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);

            image->signalRouter()->emitNotifyBatchUpdateEnded();
            image->disableUIUpdates();
        }

        KisSuspendProjectionUpdatesStrokeStrategy *m_strategy;
    };

    struct EndBatchUIUpdatesCommand : public StrokeJobCommand
    {
        EndBatchUIUpdatesCommand(KisSuspendProjectionUpdatesStrokeStrategy *strategy)
            : StrokeJobCommand(KisStrokeJobData::BARRIER),
              m_strategy(strategy) {}

        void redo() override {
            KisImageSP image = m_strategy->m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);

            image->signalRouter()->emitNotifyBatchUpdateEnded();
            m_strategy->m_d->sanityResumingFinished = true;
        }

        void undo() override {
            KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "why the heck we are undoing the last job of the stroke?!");

            m_strategy->m_d->sanityResumingFinished = false;

            KisImageSP image = m_strategy->m_d->image.toStrongRef();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);

            image->signalRouter()->emitNotifyBatchUpdateStarted();
        }

        KisSuspendProjectionUpdatesStrokeStrategy *m_strategy;
    };


    QVector<StrokeJobCommand*> executedCommands;
};

KisSuspendProjectionUpdatesStrokeStrategy::KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP image, bool suspend)
    : KisRunnableBasedStrokeStrategy(suspend ? "suspend_stroke_strategy" : "resume_stroke_strategy"),
      m_d(new Private)
{
    m_d->image = image;
    m_d->suspend = suspend;


    /**
     * Here we add a dumb INIT job so that KisStrokesQueue would know that the
     * stroke has already started or not. When the queue reaches the resume
     * stroke and starts its execution, no Lod0 can execute anymore. So all the
     * new Lod0 strokes should go to the end of the queue and wrapped into
     * their own Suspend/Resume pair.
     */
    enableJob(JOB_INIT, true);

    enableJob(JOB_DOSTROKE, true);
    enableJob(JOB_CANCEL, true);

    enableJob(JOB_SUSPEND, true, KisStrokeJobData::BARRIER);
    enableJob(JOB_RESUME, true, KisStrokeJobData::BARRIER);

    setNeedsExplicitCancel(true);
}

KisSuspendProjectionUpdatesStrokeStrategy::~KisSuspendProjectionUpdatesStrokeStrategy()
{
    qDeleteAll(m_d->executedCommands);
}

void KisSuspendProjectionUpdatesStrokeStrategy::initStrokeCallback()
{
    QVector<KisRunnableStrokeJobDataBase*> jobs;

    if (m_d->suspend) {
        jobs << new Private::UndoableData(new Private::SuspendUpdatesCommand(m_d.data()));
    } else {
        jobs << new Private::UndoableData(new Private::ResumeAndIssueGraphUpdatesCommand(m_d.data()));
        jobs << new Private::BlockUILodSync(true, this);
        jobs << new Private::UndoableData(new Private::StartBatchUIUpdatesCommand(this));
        jobs << new Private::UndoableData(new Private::EndBatchUIUpdatesCommand(this));
        jobs << new Private::BlockUILodSync(false, this);
    }

    runnableJobsInterface()->addRunnableJobs(jobs);
}

/**
 * When the Lod0 stroke is being recalculated in the background we
 * should block all the updates it issues to avoid user distraction.
 * The result of the final stroke should be shown to the user in the
 * very end when everything is fully ready. Ideally the use should not
 * notice that the image has changed :)
 *
 * (Don't mix this process with suspend/resume capabilities of a
 * single stroke. That is a different system!)
 *
 * The process of the Lod0 regeneration consists of the following:
 *
 * 1) Suspend stroke executes. It sets a special updates filter on the
 *    image. The filter blocks all the updates and saves them in an
 *    internal structure to be emitted in the future.
 *
 * 2) Lod0 strokes are being recalculated. All their updates are
 *    blocked and saved in the filter.
 *
 * 3) Resume stroke starts:
 *
 *     3.1) First it disables emitting of sigImageUpdated() so the gui
 *          will not get any update notifications.
 *
 *     3.2) Then it enables updates themselves.
 *
 *     3.3) Initiates all the updates that were requested by the Lod0
 *          stroke. The node graph is regenerated, but the GUI does
 *          not get this change.
 *
 *     3.4) Special barrier job waits for all the updates to finish
 *          and, when they are done, enables GUI notifications again.
 *
 *     3.5) In a multithreaded way emits the GUI notifications for the
 *          entire image. Multithreaded way is used to conform the
 *          double-stage update principle of KisCanvas2.
 */
void KisSuspendProjectionUpdatesStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    KisRunnableStrokeJobDataBase *runnable = dynamic_cast<KisRunnableStrokeJobDataBase*>(data);
    if (runnable) {
        runnable->run();

        if (Private::UndoableData *undoable = dynamic_cast<Private::UndoableData*>(data)) {
            Private::StrokeJobCommand *command =  undoable->m_command.take();
            m_d->executedCommands.append(command);
        }
    }
}

QList<KisStrokeJobData*> KisSuspendProjectionUpdatesStrokeStrategy::createSuspendJobsData(KisImageWSP /*image*/)
{
    return QList<KisStrokeJobData*>();
}

QList<KisStrokeJobData*> KisSuspendProjectionUpdatesStrokeStrategy::createResumeJobsData(KisImageWSP /*_image*/)
{
    return QList<KisStrokeJobData*>();
}

void KisSuspendProjectionUpdatesStrokeStrategy::Private::tryFetchUsedUpdatesFilter(KisImageSP image)
{
    KisProjectionUpdatesFilterSP filter =
        image->projectionUpdatesFilter();

    if (!filter) return;

    QSharedPointer<Private::SuspendLod0Updates> localFilter =
        filter.dynamicCast<Private::SuspendLod0Updates>();

    if (localFilter) {
        image->setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP());
        this->usedFilters.append(localFilter);
    }
}

void KisSuspendProjectionUpdatesStrokeStrategy::Private::tryIssueRecordedDirtyRequests(KisImageSP image)
{
    Q_FOREACH (QSharedPointer<Private::SuspendLod0Updates> filter, usedFilters) {
        filter->notifyUpdates(image.data());
    }
}

void KisSuspendProjectionUpdatesStrokeStrategy::cancelStrokeCallback()
{
    KisImageSP image = m_d->image.toStrongRef();
    if (!image) {
        return;
    }

    for (auto it = m_d->executedCommands.rbegin(); it != m_d->executedCommands.rend(); ++it) {
        (*it)->undo();
    }

    m_d->tryFetchUsedUpdatesFilter(image);

    if (m_d->haveDisabledGUILodSync) {
        image->signalRouter()->emitRequestLodPlanesSyncBlocked(false);
    }

    /**
     * We shouldn't emit any ad-hoc updates when cancelling the
     * stroke.  It generates weird temporary holes on the canvas,
     * making the user feel awful, thinking his image got
     * corrupted. We will just emit a common refreshGraphAsync() that
     * will do all the work in a beautiful way
     */
    if (!m_d->suspend) {
        // FIXME: optimize
        image->refreshGraphAsync();
    }
}

void KisSuspendProjectionUpdatesStrokeStrategy::suspendStrokeCallback()
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->suspend || !m_d->sanityResumingFinished);

    for (auto it = m_d->executedCommands.rbegin(); it != m_d->executedCommands.rend(); ++it) {
        (*it)->undo();
    }

    // reset all the issued updates
    m_d->updatesEpoch++;
}

void KisSuspendProjectionUpdatesStrokeStrategy::resumeStrokeCallback()
{
    QVector<KisRunnableStrokeJobDataBase*> jobs;

    Q_FOREACH (Private::StrokeJobCommand *command, m_d->executedCommands) {
        jobs << new Private::UndoableData(command);
    }
    m_d->executedCommands.clear();

    runnableJobsInterface()->addRunnableJobs(jobs);
}
