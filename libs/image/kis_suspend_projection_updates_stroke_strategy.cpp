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

    class SuspendLod0Updates : public KisProjectionUpdatesFilter
    {

        typedef QHash<KisNodeSP, QVector<QRect> > RectsHash;
    public:
        struct Request {
            Request() {}
            Request(KisNodeSP _node, QRect _rect) : node(_node), rect(_rect) {}
            KisNodeSP node;
            QRect rect;
        };
    public:
        SuspendLod0Updates()
        {
        }

        bool filter(KisImage *image, KisNode *node, const QRect &rect) {
            if (image->currentLevelOfDetail() > 0) return false;

            QMutexLocker l(&m_mutex);
            m_requestsHash[KisNodeSP(node)].append(rect);
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
                const QVector<QRect> &rects = it.value();

                QVector<QRect>::const_iterator it = rects.constBegin();
                QVector<QRect>::const_iterator end = rects.constEnd();

                QRegion region;

                for (; it != end; ++it) {
                    region += alignRect(*it, step);
                }

                Q_FOREACH (const QRect &rc, region.rects()) {
                    // FIXME: constness: port rPU to SP
                    listener->requestProjectionUpdate(const_cast<KisNode*>(node.data()), rc);

                }
            }
        }

    private:
        RectsHash m_requestsHash;
        QMutex m_mutex;
    };

    class SuspendData : public KisStrokeJobData {
    public:
        SuspendData()
            : KisStrokeJobData(SEQUENTIAL)
            {}
    };

    class ResumeAndIssueGraphUpdatesData : public KisStrokeJobData {
    public:
        ResumeAndIssueGraphUpdatesData()
            : KisStrokeJobData(SEQUENTIAL)
            {}
    };

    class UpdatesBarrierData : public KisStrokeJobData {
    public:
        UpdatesBarrierData()
            : KisStrokeJobData(BARRIER)
            {}
    };

    class IssueCanvasUpdatesData : public KisStrokeJobData {
    public:
        IssueCanvasUpdatesData(QRect _updateRect)
            : KisStrokeJobData(CONCURRENT),
              updateRect(_updateRect)
            {}
        QRect updateRect;
    };
};

KisSuspendProjectionUpdatesStrokeStrategy::KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP image, bool suspend)
    : KisSimpleStrokeStrategy(suspend ? "suspend_stroke_strategy" : "resume_stroke_strategy"),
      m_d(new Private)
{
    m_d->image = image;
    m_d->suspend = suspend;

    enableJob(JOB_DOSTROKE, true);
    enableJob(JOB_CANCEL, true);
}

KisSuspendProjectionUpdatesStrokeStrategy::~KisSuspendProjectionUpdatesStrokeStrategy()
{
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
    Private::SuspendData *suspendData = dynamic_cast<Private::SuspendData*>(data);
    Private::ResumeAndIssueGraphUpdatesData *resumeData = dynamic_cast<Private::ResumeAndIssueGraphUpdatesData*>(data);
    Private::UpdatesBarrierData *barrierData = dynamic_cast<Private::UpdatesBarrierData*>(data);
    Private::IssueCanvasUpdatesData *canvasUpdates = dynamic_cast<Private::IssueCanvasUpdatesData*>(data);

    if (suspendData) {
        m_d->image->setProjectionUpdatesFilter(
            KisProjectionUpdatesFilterSP(new Private::SuspendLod0Updates()));
    } else if (resumeData) {
        m_d->image->disableUIUpdates();
        resumeAndIssueUpdates(false);
    } else if (barrierData) {
        m_d->image->enableUIUpdates();
    } else if (canvasUpdates) {
        m_d->image->notifyProjectionUpdated(canvasUpdates->updateRect);
    }
}

QList<KisStrokeJobData*> KisSuspendProjectionUpdatesStrokeStrategy::createSuspendJobsData(KisImageWSP image)
{
    QList<KisStrokeJobData*> jobsData;
    jobsData << new Private::SuspendData();
    return jobsData;
}

QList<KisStrokeJobData*> KisSuspendProjectionUpdatesStrokeStrategy::createResumeJobsData(KisImageWSP _image)
{
    QList<KisStrokeJobData*> jobsData;

    jobsData << new Private::ResumeAndIssueGraphUpdatesData();
    jobsData << new Private::UpdatesBarrierData();

    using KritaUtils::splitRectIntoPatches;
    using KritaUtils::optimalPatchSize;
    KisImageSP image = _image;

    QVector<QRect> rects = splitRectIntoPatches(image->bounds(), optimalPatchSize());
    Q_FOREACH (const QRect &rc, rects) {
        jobsData << new Private::IssueCanvasUpdatesData(rc);
    }

    return jobsData;
}

void KisSuspendProjectionUpdatesStrokeStrategy::resumeAndIssueUpdates(bool dropUpdates)
{
    KisProjectionUpdatesFilterSP filter =
        m_d->image->projectionUpdatesFilter();

    if (!filter) return;

    Private::SuspendLod0Updates *localFilter =
        dynamic_cast<Private::SuspendLod0Updates*>(filter.data());

    if (localFilter) {
        m_d->image->setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP());

        if (!dropUpdates) {
            localFilter->notifyUpdates(m_d->image.data());
        }
    }
}

void KisSuspendProjectionUpdatesStrokeStrategy::cancelStrokeCallback()
{
    /**
     * We shouldn't emit any ad-hoc updates when cancelling the
     * stroke.  It generates weird temporary holes on the canvas,
     * making the user feel awful, thinking his image got
     * corrupted. We will just emit a common refreshGraphAsync() that
     * will do all the work in a beautiful way
     */
    resumeAndIssueUpdates(true);

    if (!m_d->suspend) {
        // FIXME: optimize
        m_d->image->refreshGraphAsync();
    }
}
