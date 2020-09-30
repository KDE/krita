/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_stroke_strategy.h"

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <krita_utils.h>
#include <kis_transaction.h>
#include <kis_paint_device_frames_interface.h>
#include <KisRunnableStrokeJobUtils.h>
#include <KisRunnableStrokeJobsInterface.h>
#include <KoCompositeOpRegistry.h>


struct KisFilterStrokeStrategy::Private {
    Private()
        : updatesFacade(0),
          secondaryTransaction(0),
          cancelSilentlyHandle(new QAtomicInt()),
          levelOfDetail(0)
    {
    }

    Private(const Private &rhs)
        : filter(rhs.filter),
          filterConfig(rhs.filterConfig),
          node(rhs.node),
          updatesFacade(rhs.updatesFacade),
          filterDevice(),
          filterDeviceBounds(),
          secondaryTransaction(0),
          progressHelper(),
          cancelSilentlyHandle(rhs.cancelSilentlyHandle),
          levelOfDetail(0)
    {
        KIS_ASSERT_RECOVER_RETURN(!rhs.filterDevice);
        KIS_ASSERT_RECOVER_RETURN(rhs.filterDeviceBounds.isEmpty());
        KIS_ASSERT_RECOVER_RETURN(!rhs.secondaryTransaction);
        KIS_ASSERT_RECOVER_RETURN(!rhs.progressHelper);
        KIS_ASSERT_RECOVER_RETURN(!rhs.levelOfDetail);
    }

    KisFilterSP filter;
    KisFilterConfigurationSP filterConfig;
    KisNodeSP node;
    KisUpdatesFacade *updatesFacade;

    KisPaintDeviceSP filterDevice;
    QRect filterDeviceBounds;
    KisTransaction *secondaryTransaction;
    QScopedPointer<KisProcessingVisitor::ProgressHelper> progressHelper;
    QSharedPointer<QAtomicInt> cancelSilentlyHandle;

    int levelOfDetail;
};


KisFilterStrokeStrategy::KisFilterStrokeStrategy(KisFilterSP filter,
                                                 KisFilterConfigurationSP filterConfig,
                                                 KisResourcesSnapshotSP resources)
    : KisPainterBasedStrokeStrategy(QLatin1String("FILTER_STROKE"),
                                    kundo2_i18n("Filter \"%1\"", filter->name()),
                                    resources,
                                    QVector<KisFreehandStrokeInfo*>(),false),
      m_d(new Private())
{
    m_d->filter = filter;
    m_d->filterConfig = filterConfig;
    m_d->node = resources->currentNode();
    m_d->updatesFacade = resources->image().data();
    m_d->secondaryTransaction = 0;
    m_d->levelOfDetail = 0;

    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

KisFilterStrokeStrategy::KisFilterStrokeStrategy(const KisFilterStrokeStrategy &rhs, int levelOfDetail)
    : KisPainterBasedStrokeStrategy(rhs, levelOfDetail),
      m_d(new Private(*rhs.m_d))
{
    // only non-started transaction are allowed
    KIS_ASSERT_RECOVER_NOOP(!m_d->secondaryTransaction);
    m_d->levelOfDetail = levelOfDetail;
}

KisFilterStrokeStrategy::~KisFilterStrokeStrategy()
{
    delete m_d;
}

void KisFilterStrokeStrategy::initStrokeCallback()
{
    KisPainterBasedStrokeStrategy::initStrokeCallback();

    KisPaintDeviceSP dev = targetDevice();
    m_d->filterDeviceBounds = dev->extent();

    if (m_d->filter->needsTransparentPixels(m_d->filterConfig.data(), dev->colorSpace())) {
        m_d->filterDeviceBounds |= dev->defaultBounds()->bounds();
    }

    if (activeSelection() ||
        (dev->colorSpace() != dev->compositionSourceColorSpace() &&
         *dev->colorSpace() != *dev->compositionSourceColorSpace())) {

        m_d->filterDevice = dev->createCompositionSourceDevice(dev);
        m_d->secondaryTransaction = new KisTransaction(m_d->filterDevice);

        if (activeSelection()) {
            m_d->filterDeviceBounds &= activeSelection()->selectedRect();
        }
    } else {
        m_d->filterDevice = dev;
    }

    m_d->progressHelper.reset(new KisProcessingVisitor::ProgressHelper(m_d->node));
}

/*
 *
 * TEMP NOTE DELETE LATER:
 * "KisFilterStrokeStrategy::Data should only have a "frameId" paramenter. No `concurrent` or `_processRect`.
 * The latter two variables should be calculated internally in doStrokeCallback,
 * and after that a set of "mutated jobs" should be generated" - Dmitry
 *
 */
void KisFilterStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    ExtraCleanUpUpdates *cleanup = dynamic_cast<ExtraCleanUpUpdates*>(data);

    KisRunnableStrokeJobData *jobData = dynamic_cast<KisRunnableStrokeJobData*>(data);

    if (d) {
        using namespace KritaUtils;
        QVector<KisRunnableStrokeJobData*> jobs;

        const QRect applyRect = m_d->node->image()->bounds() | m_d->node->exactBounds(); // DO WE NEED THIS?
        const QRect processRect = m_d->filter->changedRect(applyRect, m_d->filterConfig.data(), 0);
        const bool useSpecificFrameID = d->m_frameID != -1;
        const int frameID = d->m_frameID;

        //If we're using a specific frame ID, copy the contents of that frameID first...
        addJobSequential(jobs, [this, frameID, useSpecificFrameID](){
            if (useSpecificFrameID && m_d->node->original()) {
                m_d->filterDevice = new KisPaintDevice(*m_d->filterDevice, KritaUtils::CopySnapshot);
                m_d->node->original()->framesInterface()->writeFrameToDevice(frameID, m_d->filterDevice);
            }
        });

        if (m_d->filter->supportsThreading()) {
            // Split stroke into patches...
            QSize size = KritaUtils::optimalPatchSize();
            QVector<QRect> patches = KritaUtils::splitRectIntoPatches(processRect, size);

            Q_FOREACH (const QRect &patch, patches) {
                addJobConcurrent(jobs, [this, patch](){
                    m_d->filter->processImpl(m_d->filterDevice, patch,
                                             m_d->filterConfig.data(),
                                             m_d->progressHelper->updater());
                });
            }
        } else {
            addJobSequential(jobs, [this, processRect](){
                m_d->filter->processImpl(m_d->filterDevice, processRect,
                                         m_d->filterConfig.data(),
                                         m_d->progressHelper->updater());
            });
        }

        if (useSpecificFrameID) {

            addJobSequential(jobs, [this, applyRect, frameID](){
                if (!m_d->filterDeviceBounds.intersects(
                        m_d->filter->neededRect(applyRect, m_d->filterConfig.data(), m_d->levelOfDetail))) {
                    return;
                }

                if (m_d->secondaryTransaction) {

                    KisPaintDeviceSP target = new KisPaintDevice(*target, KritaUtils::CopySnapshot);
                    targetDevice()->framesInterface()->writeFrameToDevice(frameID, target);
                    KisPainter::copyAreaOptimized(applyRect.topLeft(), m_d->filterDevice, target, applyRect, activeSelection());
                    targetDevice()->framesInterface()->uploadFrame(frameID, target);

                    // Free memory
                    m_d->filterDevice->clear(applyRect);
                } else {

                    targetDevice()->framesInterface()->uploadFrame(frameID, m_d->filterDevice);
                }

                m_d->node->setDirty(applyRect);
            });

        } else {

            addJobSequential(jobs, [this, applyRect](){
                if (!m_d->filterDeviceBounds.intersects(
                        m_d->filter->neededRect(applyRect, m_d->filterConfig.data(), m_d->levelOfDetail))) {
                    return;
                }

                if (m_d->secondaryTransaction) {
                    KisPainter::copyAreaOptimized(applyRect.topLeft(), m_d->filterDevice, targetDevice(), applyRect, activeSelection());

                    // Free memory
                    m_d->filterDevice->clear(applyRect);
                }

                m_d->node->setDirty(applyRect);
            });
        }

        runnableJobsInterface()->addRunnableJobs(jobs);
        m_d->node->setDirty(rc);

    } else if (cleanup) {
        m_d->node->setDirty(cleanup->rects);
    } else if (dynamic_cast<IdleBarrierData*>(data)) {
        /* noop, just delete that */
    } else if (jobData) {
        jobData->run();
    } else {
        qFatal("KisFilterStrokeStrategy: job type is not known");
    }
}

void KisFilterStrokeStrategy::cancelStrokeCallback()
{
    delete m_d->secondaryTransaction;
    m_d->filterDevice = 0;

    const bool shouldCancelSilently = *m_d->cancelSilentlyHandle;

    if (shouldCancelSilently) {
        m_d->updatesFacade->disableDirtyRequests();
    }

    KisPainterBasedStrokeStrategy::cancelStrokeCallback();

    if (shouldCancelSilently) {
        m_d->updatesFacade->enableDirtyRequests();
    }
}

void KisFilterStrokeStrategy::finishStrokeCallback()
{
    delete m_d->secondaryTransaction;
    m_d->filterDevice = 0;

    KisPainterBasedStrokeStrategy::finishStrokeCallback();
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
