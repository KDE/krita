/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_stroke_strategy.h"

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <krita_utils.h>
#include <kis_layer_utils.h>
#include <kis_raster_keyframe_channel.h>
#include <kis_transaction.h>
#include <kis_paint_device_frames_interface.h>
#include <KisRunnableStrokeJobUtils.h>
#include <KisRunnableStrokeJobsInterface.h>
#include <KoCompositeOpRegistry.h>
#include "kis_painter.h"

struct KisFilterStrokeStrategy::Private {
    Private()
        : updatesFacade(0),
          cancelSilentlyHandle(new QAtomicInt()),
          levelOfDetail(0)
    {
    }

    Private(const Private &rhs)
        : filter(rhs.filter),
          filterConfig(rhs.filterConfig),
          node(rhs.node),
          targetDevice(rhs.targetDevice),
          activeSelection(rhs.activeSelection),
          image(rhs.image),
          updatesFacade(rhs.updatesFacade),
          filterDevice(),
          filterDeviceBounds(),
          progressHelper(),
          cancelSilentlyHandle(rhs.cancelSilentlyHandle),
          levelOfDetail(0)
    {
        KIS_ASSERT_RECOVER_RETURN(!rhs.filterDevice);
        KIS_ASSERT_RECOVER_RETURN(rhs.filterDeviceBounds.isEmpty());
        KIS_ASSERT_RECOVER_RETURN(!rhs.progressHelper);
        KIS_ASSERT_RECOVER_RETURN(!rhs.levelOfDetail);
    }

    KisFilterSP filter;
    KisFilterConfigurationSP filterConfig;
    KisNodeSP node;
    KisPaintDeviceSP targetDevice;
    KisSelectionSP activeSelection;
    KisImageSP image;
    KisUpdatesFacade *updatesFacade;

    KisTransaction* workingTransaction;

    KisPaintDeviceSP filterDevice;
    QRect filterDeviceBounds;

    QScopedPointer<KisProcessingVisitor::ProgressHelper> progressHelper;
    QSharedPointer<QAtomicInt> cancelSilentlyHandle;

    int levelOfDetail;
};


KisFilterStrokeStrategy::KisFilterStrokeStrategy(KisFilterSP filter,
                                                 KisFilterConfigurationSP filterConfig,
                                                 KisResourcesSnapshotSP resources)
    : KisStrokeStrategyUndoCommandBased(kundo2_i18n("Filter \"%1\"", filter->name()),
                                        false,
                                        resources->image().data())
    , m_d(new Private())
{
    m_d->filter = filter;
    m_d->filterConfig = filterConfig;
    m_d->node = resources->currentNode();
    m_d->targetDevice = resources->currentNode()->paintDevice();
    m_d->activeSelection = resources->activeSelection();
    m_d->image = resources->image();
    m_d->updatesFacade = resources->image().data();
    m_d->levelOfDetail = 0;

    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_INIT);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
}

KisFilterStrokeStrategy::KisFilterStrokeStrategy(const KisFilterStrokeStrategy &rhs, int levelOfDetail)
    : KisStrokeStrategyUndoCommandBased(rhs)
    , m_d(new Private(*rhs.m_d))
{
    m_d->levelOfDetail = levelOfDetail;
}

KisFilterStrokeStrategy::~KisFilterStrokeStrategy()
{
    delete m_d;
}

void KisFilterStrokeStrategy::initStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::initStrokeCallback();

    m_d->progressHelper.reset(new KisProcessingVisitor::ProgressHelper(m_d->node));
}


void KisFilterStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    FilterJobData *filterFrameData = dynamic_cast<FilterJobData*>(data);
    KisRunnableStrokeJobData *jobData = dynamic_cast<KisRunnableStrokeJobData*>(data);
    ExtraCleanUpUpdates *cleanup = dynamic_cast<ExtraCleanUpUpdates*>(data);

    if (filterFrameData) { // Populate list of jobs for filter application...

        using namespace KritaUtils;
        QVector<KisRunnableStrokeJobData*> jobs;

        const QRect applyRect = m_d->node->image()->bounds();
        const QRect processRect = m_d->filter->changedRect(applyRect, m_d->filterConfig.data(), 0);
        const bool shouldSwitchTime = filterFrameData->frameTime != -1 && filterFrameData->frameTime != KisLayerUtils::fetchLayerActiveFrameTime(m_d->node);
        const int frameTime = filterFrameData->frameTime;

        KisLayerUtils::SwitchFrameCommand::SharedStorageSP storage( new KisLayerUtils::SwitchFrameCommand::SharedStorage() );

        //Otherwise, a simple snapshot should suffice..
        m_d->filterDevice = new KisPaintDevice(*m_d->targetDevice);

        addJobSequential(jobs, [this, shouldSwitchTime, frameTime, storage](){
            // Switch time if necessary..
            if (shouldSwitchTime) {
                runAndSaveCommand( toQShared( new KisLayerUtils::SwitchFrameCommand(m_d->image, frameTime, false, storage) )
                                   , KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
            }

            // Copy snapshot of targetDevice for filter processing..
            m_d->filterDevice = new KisPaintDevice(*m_d->targetDevice);

            //If we're dealing with some kind of transparency mask, we will create a compositionSourceDevice instead.
            //  Carry over from commit ca810f85 ...
            if (m_d->activeSelection ||
                    (m_d->targetDevice->colorSpace() != m_d->targetDevice->compositionSourceColorSpace() &&
                    *m_d->targetDevice->colorSpace() != *m_d->targetDevice->compositionSourceColorSpace())) {
                m_d->filterDevice = m_d->targetDevice->createCompositionSourceDevice(m_d->targetDevice);
                if (m_d->activeSelection) {
                    m_d->filterDeviceBounds &= m_d->activeSelection->selectedRect();
                }
            }

            //Update all necessary rect data based on contents of frame..
            m_d->filterDeviceBounds = m_d->filterDevice->extent();

            if (m_d->filter->needsTransparentPixels(m_d->filterConfig.data(), m_d->targetDevice->colorSpace())) {
                m_d->filterDeviceBounds |= m_d->targetDevice->defaultBounds()->bounds();
            }
        });

        if (m_d->filter->supportsThreading()) {
            // Split stroke into patches...
            QSize size = KritaUtils::optimalPatchSize();
            QVector<QRect> patches = KritaUtils::splitRectIntoPatches(processRect, size);

            Q_FOREACH (const QRect &patch, patches) {
                // Filter subrect concurently
                addJobConcurrent(jobs, [this, patch](){
                    if (patch.intersects(m_d->filterDeviceBounds)) {
                        m_d->filter->processImpl(m_d->filterDevice, patch,
                                                 m_d->filterConfig.data(),
                                                 m_d->progressHelper->updater());
                    }
                });
            }
        } else {

            //Filter whole paint device.
            addJobSequential(jobs, [this, processRect](){
                m_d->filter->processImpl(m_d->filterDevice, processRect,
                                         m_d->filterConfig.data(),
                                         m_d->progressHelper->updater());
            });
        }

        addJobSequential(jobs, [this, applyRect, shouldSwitchTime](){
            if (!m_d->filterDeviceBounds.intersects(
                    m_d->filter->neededRect(applyRect, m_d->filterConfig.data(), m_d->levelOfDetail))) {
                return;
            }

            // Make a transaction, change the target device, and "end" transaction.
            // Should be useful for undoing later.
            m_d->workingTransaction = new KisTransaction(m_d->targetDevice);
            KisPainter::copyAreaOptimized(applyRect.topLeft(), m_d->filterDevice, m_d->targetDevice, applyRect, m_d->activeSelection);
            runAndSaveCommand( toQShared(m_d->workingTransaction->endAndTake()), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL );
            delete m_d->workingTransaction;
            m_d->workingTransaction = nullptr;

            //If we're talking about the "current" / visible frame, mark rect as dirty.
            if (!shouldSwitchTime) {
                m_d->node->setDirty(applyRect);
            }
        });

        addJobSequential(jobs, [this, shouldSwitchTime, frameTime, storage](){
            if (shouldSwitchTime) {
                runAndSaveCommand( toQShared( new KisLayerUtils::SwitchFrameCommand(m_d->image, frameTime, true, storage) )
                                   , KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
            }
        });

        runnableJobsInterface()->addRunnableJobs(jobs);

    } else if (cleanup) {
        m_d->node->setDirty(cleanup->rects);
    } else if (dynamic_cast<IdleBarrierData*>(data)) {
        /* noop, just delete that */
    } else if (jobData) {
        jobData->run();
    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
//        qFatal("KisFilterStrokeStrategy: job type is not known");
    }
}

void KisFilterStrokeStrategy::cancelStrokeCallback()
{
    m_d->filterDevice = 0;

    const bool shouldCancelSilently = *m_d->cancelSilentlyHandle;

    if (shouldCancelSilently) {
        m_d->updatesFacade->disableDirtyRequests();
    }

    KisStrokeStrategyUndoCommandBased::cancelStrokeCallback();

    if (shouldCancelSilently) {
        m_d->updatesFacade->enableDirtyRequests();
    }
}

void KisFilterStrokeStrategy::finishStrokeCallback()
{
    m_d->filterDevice = 0;

    // KisPainterBasedStrokeStrategy::finishStrokeCallback() saves an undo
    // state using KisTransaction for current active keyframe, not the frameIDs that have
    // rendered in a given stroke. Meaning, we can only undo the current active frame atm...
    KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
}

KisStrokeStrategy* KisFilterStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!m_d->filter->supportsLevelOfDetail(m_d->filterConfig.data(), levelOfDetail)) return 0;
    if (!m_d->node->supportsLodPainting()) return 0;

    KisFilterStrokeStrategy *clone = new KisFilterStrokeStrategy(*this, levelOfDetail);
    return clone;
}

QSharedPointer<QAtomicInt> KisFilterStrokeStrategy::cancelSilentlyHandle() const
{
    return m_d->cancelSilentlyHandle;
}
