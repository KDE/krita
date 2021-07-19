/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_regenerate_frame_stroke_strategy.h"

#include <KisRegion.h>
#include "kis_image_interfaces.h"
#include "kis_image_animation_interface.h"
#include "kis_node.h"
#include "kis_image.h"
#include "krita_utils.h"

#include "kis_full_refresh_walker.h"
#include "kis_async_merger.h"
#include "kis_projection_updates_filter.h"


struct KisRegenerateFrameStrokeStrategy::Private
{
    Type type;
    int frameId;
    int previousFrameId;
    KisRegion dirtyRegion;
    KisImageAnimationInterface *interface;
    QStack<KisProjectionUpdatesFilterSP> prevUpdatesFilters;

    class Data : public KisStrokeJobData {
    public:
        Data(KisNodeSP _root, const QRect &_rect, const QRect &_cropRect)
            : KisStrokeJobData(CONCURRENT),
              root(_root), rect(_rect), cropRect(_cropRect)
            {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            Q_UNUSED(levelOfDetail);
            return new KisStrokeJobData(CONCURRENT);
        }

        KisNodeSP root;
        QRect rect;
        QRect cropRect;
    };

    void saveAndResetUpdatesFilter() {
        KisImageSP image = interface->image().toStrongRef();
        if (!image) {
            return;
        }

        while (KisProjectionUpdatesFilterCookie cookie = image->currentProjectionUpdatesFilter()) {
            prevUpdatesFilters.push(image->removeProjectionUpdatesFilter(cookie));
        }
    }

    void restoreUpdatesFilter() {
        KisImageSP image = interface->image().toStrongRef();
        if (!image) {
            return;
        }

        while (!prevUpdatesFilters.isEmpty()) {
            image->addProjectionUpdatesFilter(prevUpdatesFilters.pop());
        }
    }
};

KisRegenerateFrameStrokeStrategy::KisRegenerateFrameStrokeStrategy(int frameId,
                                                                   const KisRegion &dirtyRegion,
                                                                   bool isCancellable,
                                                                   KisImageAnimationInterface *interface)
    : KisSimpleStrokeStrategy(QLatin1String("regenerate_external_frame_stroke")),
      m_d(new Private)
{
    m_d->type = EXTERNAL_FRAME;

    m_d->frameId = frameId;
    m_d->dirtyRegion = dirtyRegion;
    m_d->interface = interface;

    enableJob(JOB_INIT);
    enableJob(JOB_FINISH, true, KisStrokeJobData::BARRIER);
    enableJob(JOB_CANCEL, true, KisStrokeJobData::BARRIER);

    enableJob(JOB_DOSTROKE);

    enableJob(JOB_SUSPEND);
    enableJob(JOB_RESUME);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(isCancellable);
}

KisRegenerateFrameStrokeStrategy::KisRegenerateFrameStrokeStrategy(KisImageAnimationInterface *interface)
    : KisSimpleStrokeStrategy(QLatin1String("regenerate_current_frame_stroke"), kundo2_i18n("Render Animation")),
      m_d(new Private)
{
    m_d->type = CURRENT_FRAME;

    m_d->frameId = 0;
    m_d->dirtyRegion = KisRegion();
    m_d->interface = interface;

    enableJob(JOB_INIT);
    enableJob(JOB_FINISH, true, KisStrokeJobData::BARRIER);
    enableJob(JOB_CANCEL, true, KisStrokeJobData::BARRIER);

    enableJob(JOB_SUSPEND);
    enableJob(JOB_RESUME);

    // switching frames is a distinct user action, so it should
    // cancel the playback or any action easily
    setRequestsOtherStrokesToEnd(true);
    setClearsRedoOnStart(false);
}

KisRegenerateFrameStrokeStrategy::~KisRegenerateFrameStrokeStrategy()
{
}

void KisRegenerateFrameStrokeStrategy::initStrokeCallback()
{
    KisImageSP image = m_d->interface->image().toStrongRef();
    if (!image) {
        return;
    }
    if (m_d->type == EXTERNAL_FRAME) {
        m_d->saveAndResetUpdatesFilter();
        image->disableUIUpdates();
        m_d->interface->saveAndResetCurrentTime(m_d->frameId, &m_d->previousFrameId);
    } else if (m_d->type == CURRENT_FRAME) {
        m_d->interface->blockFrameInvalidation(true);
        m_d->interface->updatesFacade()->refreshGraphAsync(KisNodeSP());
    }
}

void KisRegenerateFrameStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Private::Data *d = dynamic_cast<Private::Data*>(data);
    KIS_ASSERT(d);
    KIS_ASSERT(!m_d->dirtyRegion.isEmpty());
    KIS_ASSERT(m_d->type == EXTERNAL_FRAME);

    const bool skipNonRenderableNodes = m_d->type == EXTERNAL_FRAME;
    KisBaseRectsWalkerSP walker = new KisFullRefreshWalker(d->cropRect, skipNonRenderableNodes);
    walker->collectRects(d->root, d->rect);

    KisAsyncMerger merger;
    merger.startMerge(*walker);
}

void KisRegenerateFrameStrokeStrategy::finishStrokeCallback()
{
    KisImageSP image = m_d->interface->image().toStrongRef();
    if (!image) {
        return;
    }
    if (m_d->type == EXTERNAL_FRAME) {
        m_d->interface->notifyFrameReady();
        m_d->interface->restoreCurrentTime(&m_d->previousFrameId);
        image->enableUIUpdates();
        m_d->restoreUpdatesFilter();
    } else if (m_d->type == CURRENT_FRAME) {
        m_d->interface->blockFrameInvalidation(false);
    }
}

void KisRegenerateFrameStrokeStrategy::cancelStrokeCallback()
{
    KisImageSP image = m_d->interface->image().toStrongRef();
    if (!image) {
        return;
    }
    if (m_d->type == EXTERNAL_FRAME) {
        m_d->interface->notifyFrameCancelled();
        m_d->interface->restoreCurrentTime(&m_d->previousFrameId);
        image->enableUIUpdates();
        m_d->restoreUpdatesFilter();
    } else if (m_d->type == CURRENT_FRAME) {
        m_d->interface->blockFrameInvalidation(false);
    }
}

KisStrokeStrategy* KisRegenerateFrameStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);

    /**
     * We need to regenerate animation frames on LodN level only if
     * we are processing current frame. Return dummy stroke otherwise
     */
    return m_d->type == CURRENT_FRAME ?
        new KisRegenerateFrameStrokeStrategy(m_d->interface) :
        new KisSimpleStrokeStrategy(QLatin1String("dumb-lodn-KisRegenerateFrameStrokeStrategy"));
}

void KisRegenerateFrameStrokeStrategy::suspendStrokeCallback()
{
    KisImageSP image = m_d->interface->image().toStrongRef();
    if (!image) {
        return;
    }
    if (m_d->type == EXTERNAL_FRAME) {
        m_d->interface->restoreCurrentTime(&m_d->previousFrameId);
        image->enableUIUpdates();
        m_d->restoreUpdatesFilter();
    } else if (m_d->type == CURRENT_FRAME) {
        m_d->interface->blockFrameInvalidation(false);
    }
}

void KisRegenerateFrameStrokeStrategy::resumeStrokeCallback()
{
    KisImageSP image = m_d->interface->image().toStrongRef();
    if (!image) {
        return;
    }
    if (m_d->type == EXTERNAL_FRAME) {
        m_d->saveAndResetUpdatesFilter();
        image->disableUIUpdates();
        m_d->interface->saveAndResetCurrentTime(m_d->frameId, &m_d->previousFrameId);
    } else if (m_d->type == CURRENT_FRAME) {
        m_d->interface->blockFrameInvalidation(true);
    }
}

QList<KisStrokeJobData*> KisRegenerateFrameStrokeStrategy::createJobsData(KisImageWSP _image)
{
    using KritaUtils::splitRectIntoPatches;
    using KritaUtils::optimalPatchSize;
    KisImageSP image = _image;

    const QRect cropRect = image->bounds();
    QVector<QRect> rects = splitRectIntoPatches(image->bounds(), optimalPatchSize());
    QList<KisStrokeJobData*> jobsData;

    Q_FOREACH (const QRect &rc, rects) {
        jobsData << new Private::Data(image->root(), rc, cropRect);
    }

    return jobsData;
}
