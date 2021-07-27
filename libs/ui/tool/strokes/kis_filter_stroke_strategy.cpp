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
#include "kis_image_config.h"
#include "kis_image_animation_interface.h"
#include "kis_painter.h"
#include <commands_new/KisDisableDirtyRequestsCommand.h>


struct KisFilterStrokeStrategy::Private {
    Private()
        : updatesFacade(0),
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
          levelOfDetail(0),
          cancelledUpdates(rhs.cancelledUpdates)
    {
        KIS_ASSERT_RECOVER_RETURN(!rhs.levelOfDetail);
    }

    KisFilterSP filter;
    KisFilterConfigurationSP filterConfig;
    KisNodeSP node;
    KisPaintDeviceSP targetDevice;
    KisSelectionSP activeSelection;
    KisImageSP image;
    KisUpdatesFacade *updatesFacade;
    int levelOfDetail;

    ExternalCancelUpdatesStorageSP cancelledUpdates;
    QRect nextExternalUpdateRect;
    bool hasBeenLodCloned = false;

};

struct SubTaskSharedData {

    SubTaskSharedData(KisImageSP image, KisNodeSP node, int levelOfDetail,
                      KisSelectionSP selection, KisFilterSP filter, KisFilterConfigurationSP config,
                      KisFilterStrokeStrategy::FilterJobData* filterFrameData )
        : m_image(image)
        , m_node(node)
        , m_levelOfDetail(levelOfDetail)
        , m_targetDevice(node->paintDevice())
        , m_selection(selection)
        , m_filter(filter)
        , m_filterConfig(config)
        , m_storage(new KisLayerUtils::SwitchFrameCommand::SharedStorage()){

        m_frameTime = filterFrameData->frameTime;
        m_shouldSwitchTime = filterFrameData->frameTime != -1;

        m_shouldRedraw = !m_shouldSwitchTime || filterFrameData->frameTime == KisLayerUtils::fetchLayerActiveRasterFrameTime(m_node);
    }

    ~SubTaskSharedData(){}


    KisImageSP image() { return m_image; }

    KisNodeSP node() { return m_node; }

    int levelOfDetail() { return m_levelOfDetail; }

    KisPaintDeviceSP targetDevice() { return m_targetDevice; }

    KisSelectionSP selection() { return m_selection; }

    KisFilterSP filter() { return m_filter; }

    KisFilterConfigurationSP filterConfig() { return m_filterConfig; }

    int frameTime() { return m_frameTime; }

    bool shouldSwitchTime() { return m_shouldSwitchTime; }

    bool shouldRedraw() { return m_shouldRedraw; }

    KisLayerUtils::SwitchFrameCommand::SharedStorageSP storage() { return m_storage; }

public:
    KisPaintDeviceSP filterDevice;
    QRect filterDeviceBounds;
    QSharedPointer<KisTransaction> filterDeviceTransaction;
    QRect processRect;

private:
    KisImageSP m_image;
    KisNodeSP m_node;
    int m_levelOfDetail;
    KisPaintDeviceSP m_targetDevice;
    KisSelectionSP m_selection;
    KisFilterSP m_filter;
    KisFilterConfigurationSP m_filterConfig;
    bool m_shouldSwitchTime;
    bool m_shouldRedraw;
    int m_frameTime;
    KisLayerUtils::SwitchFrameCommand::SharedStorageSP m_storage;

};


KisFilterStrokeStrategy::KisFilterStrokeStrategy(KisFilterSP filter, KisFilterConfigurationSP filterConfig, KisResourcesSnapshotSP resources)
    : KisFilterStrokeStrategy(filter, filterConfig, resources, ExternalCancelUpdatesStorageSP())
{
}

KisFilterStrokeStrategy::KisFilterStrokeStrategy(KisFilterSP filter,
                                                 KisFilterConfigurationSP filterConfig,
                                                 KisResourcesSnapshotSP resources,
                                                 ExternalCancelUpdatesStorageSP externalCancelUpdatesStorage)
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
    m_d->cancelledUpdates = externalCancelUpdatesStorage;

    setNeedsExplicitCancel(true);
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

    qSwap(m_d->nextExternalUpdateRect, m_d->cancelledUpdates->updateRect);
    KisLodTransform t(m_d->levelOfDetail);
    m_d->nextExternalUpdateRect = t.map(m_d->nextExternalUpdateRect);
}


void KisFilterStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    FilterJobData *filterFrameData = dynamic_cast<FilterJobData*>(data);
    KisRunnableStrokeJobData *jobData = dynamic_cast<KisRunnableStrokeJobData*>(data);

    if (filterFrameData) { // Populate list of jobs for filter application...

        using namespace KritaUtils;
        QVector<KisRunnableStrokeJobData*> jobs;

        QSharedPointer<SubTaskSharedData> shared( new SubTaskSharedData(m_d->image, m_d->node, m_d->levelOfDetail,
                                                                        m_d->activeSelection, m_d->filter, m_d->filterConfig, filterFrameData) );
        QSharedPointer<KisProcessingVisitor::ProgressHelper> progress( new KisProcessingVisitor::ProgressHelper(m_d->node) );
        addJobSequential(jobs, [this, shared, progress](){
            // Switch time if necessary..
            if (shared->shouldSwitchTime()) {
                runAndSaveCommand( toQShared( new KisLayerUtils::SwitchFrameCommand(shared->image(), shared->frameTime(), false, shared->storage()) )
                                   , KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
            }

            // Copy snapshot of targetDevice for filter processing..
            shared->filterDevice = new KisPaintDevice(*shared->targetDevice());

            // Update all necessary rect data based on contents of frame..
            shared->filterDeviceBounds = shared->filterDevice->exactBounds();

            if (shared->filter()->needsTransparentPixels(shared->filterConfig().data(), shared->targetDevice()->colorSpace())) {
                shared->filterDeviceBounds |= shared->targetDevice()->defaultBounds()->bounds();
            }

            shared->processRect = shared->filter()->changedRect(shared->filterDeviceBounds, shared->filterConfig(), shared->levelOfDetail());
            shared->processRect &= shared->targetDevice()->defaultBounds()->bounds();

            //If we're dealing with some kind of transparency mask, we will create a compositionSourceDevice instead.
            //  Carry over from commit ca810f85 ...
            if (shared->selection() ||
                    (shared->targetDevice()->colorSpace() != shared->targetDevice()->compositionSourceColorSpace() &&
                    *shared->targetDevice()->colorSpace() != *shared->targetDevice()->compositionSourceColorSpace())) {

                shared->filterDevice = shared->targetDevice()->createCompositionSourceDevice(shared->targetDevice());

                if (shared->selection()) {
                    shared->filterDeviceBounds &= shared->selection()->selectedRect();
                }
            }

            // Filter device needs a transaction to prevent grid-patch artifcacts from multithreaded read/write.
            shared->filterDeviceTransaction.reset(new KisTransaction(shared->filterDevice));


            // Actually process the device

            QVector<KisRunnableStrokeJobData*> processJobs;

            if (shared->filter()->supportsThreading()) {
                // Split stroke into patches...
                QSize size = KritaUtils::optimalPatchSize();
                QVector<QRect> patches = KritaUtils::splitRectIntoPatches(shared->processRect, size);

                Q_FOREACH (const QRect &patch, patches) {
                    addJobConcurrent(processJobs, [patch, shared, progress](){
                        shared->filter()->processImpl(shared->filterDevice, patch,
                                                      shared->filterConfig().data(),
                                                      progress->updater());
                    });
                }
            } else {
                addJobSequential(processJobs, [shared, progress](){
                    shared->filter()->processImpl(shared->filterDevice, shared->processRect,
                                             shared->filterConfig().data(),
                                             progress->updater());
                });
            }

            runnableJobsInterface()->addRunnableJobs(processJobs);

        });

        addJobSequential(jobs, [this, shared](){
            // We will first apply the transaction to the temporary filterDevice
            runAndSaveCommand(toQShared(shared->filterDeviceTransaction->endAndTake()), KisStrokeJobData::BARRIER, KisStrokeJobData::NORMAL);
            shared->filterDeviceTransaction.reset();

            if (!shared->filterDeviceBounds.intersects(
                    shared->filter()->neededRect(shared->processRect, shared->filterConfig().data(), shared->levelOfDetail()))) {
                return;
            }

            // Make a transaction, change the target device, and "end" transaction.
            // Should be useful for undoing later.
            QScopedPointer<KisTransaction> workingTransaction( new KisTransaction(shared->targetDevice(), AUTOKEY_DISABLED) );
            KisPainter::copyAreaOptimized(shared->processRect.topLeft(), shared->filterDevice, shared->targetDevice(), shared->processRect, shared->selection());
            runAndSaveCommand( toQShared(workingTransaction->endAndTake()), KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE );

            if (shared->shouldRedraw()) {
                QRect extraUpdateRect;
                qSwap(extraUpdateRect, m_d->nextExternalUpdateRect);

                shared->node()->setDirty(shared->processRect | extraUpdateRect);

               /**
                * Save the last update to be able to restore the
                * original state on the cancellation (even when
                * the cancellation step is explicitly prohibited)
                */
                m_d->nextExternalUpdateRect = shared->processRect;
            }
        });

        addJobSequential(jobs, [this, shared](){
            shared->image()->animationInterface()->invalidateFrame(shared->frameTime(), shared->node());
            if (shared->shouldSwitchTime()) {
                runAndSaveCommand( toQShared( new KisLayerUtils::SwitchFrameCommand(shared->image(), shared->frameTime(), true, shared->storage()) )
                                   , KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
            }
        });

        runnableJobsInterface()->addRunnableJobs(jobs);

    } else if (dynamic_cast<IdleBarrierData*>(data)) {
        /* noop, just delete that */
    } else if (jobData) {
        jobData->run();
    } else {
        KisStrokeStrategyUndoCommandBased::doStrokeCallback(data);
    }
}

void KisFilterStrokeStrategy::cancelStrokeCallback()
{
    using namespace KritaUtils;

    const bool shouldIssueCancellationUpdates = m_d->cancelledUpdates->shouldIssueCancellationUpdates;

    QVector<KisStrokeJobData *> jobs;

    jobs << new Data(toQShared(new KisDisableDirtyRequestsCommand(m_d->updatesFacade, KisDisableDirtyRequestsCommand::INITIALIZING)));
    KisStrokeStrategyUndoCommandBased::cancelStrokeCallbackImpl(jobs);
    jobs << new Data(toQShared(new KisDisableDirtyRequestsCommand(m_d->updatesFacade, KisDisableDirtyRequestsCommand::FINALIZING)));

    if (shouldIssueCancellationUpdates) {
        addJobSequential(jobs, [this] () {
            QRect updateRect =
                m_d->cancelledUpdates->updateRect |
                m_d->nextExternalUpdateRect;

            if (m_d->levelOfDetail <= 0) {
                updateRect |= m_d->cancelledUpdates->cancelledLod0UpdateRect;
            }

            if (!updateRect.isEmpty()) {
                m_d->node->setDirty(updateRect);
            }
        });
    } else if (!m_d->nextExternalUpdateRect.isEmpty()) {

        KisLodTransform t(m_d->levelOfDetail);
        m_d->cancelledUpdates->updateRect = t.mapInverted(m_d->nextExternalUpdateRect);

        if (!m_d->hasBeenLodCloned && m_d->levelOfDetail <= 0) {
            m_d->cancelledUpdates->cancelledLod0UpdateRect = m_d->nextExternalUpdateRect;
        }
    }

    addMutatedJobs(jobs);
}

void KisFilterStrokeStrategy::finishStrokeCallback()
{
    KisStrokeStrategyUndoCommandBased::finishStrokeCallback();
}

KisStrokeStrategy* KisFilterStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!m_d->filter->supportsLevelOfDetail(m_d->filterConfig.data(), levelOfDetail)) return 0;
    if (!m_d->node->supportsLodPainting()) return 0;

    KisFilterStrokeStrategy *clone = new KisFilterStrokeStrategy(*this, levelOfDetail);
    m_d->hasBeenLodCloned = true;
    return clone;
}
