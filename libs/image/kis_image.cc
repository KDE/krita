/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_image.h"

#include <KoConfig.h> // WORDS_BIGENDIAN

#include <stdlib.h>
#include <math.h>

#include <QImage>
#include <QPainter>
#include <QSize>
#include <QDateTime>
#include <QRect>
#include <QRegion>
#include <QtConcurrent>

#include <klocalizedstring.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include <KoCompositeOpRegistry.h>
#include "KisProofingConfiguration.h"

#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_change_profile_visitor.h"
#include "kis_colorspace_convert_visitor.h"
#include "kis_count_visitor.h"
#include "kis_filter_strategy.h"
#include "kis_group_layer.h"
#include "commands/kis_image_commands.h"
#include "kis_layer.h"
#include "kis_meta_data_merge_strategy_registry.h"
#include "kis_name_server.h"
#include "kis_paint_layer.h"
#include "kis_projection_leaf.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_meta_data_merge_strategy.h"
#include "kis_memory_statistics_server.h"

#include "kis_image_config.h"
#include "kis_update_scheduler.h"
#include "kis_image_signal_router.h"
#include "kis_image_animation_interface.h"
#include "kis_stroke_strategy.h"
#include "kis_simple_stroke_strategy.h"
#include "kis_image_barrier_locker.h"


#include "kis_undo_stores.h"
#include "kis_legacy_undo_adapter.h"
#include "kis_post_execution_undo_adapter.h"

#include "kis_transform_worker.h"
#include "kis_processing_applicator.h"
#include "processing/kis_crop_processing_visitor.h"
#include "processing/kis_crop_selections_processing_visitor.h"
#include "processing/kis_transform_processing_visitor.h"
#include "commands_new/kis_image_resize_command.h"
#include "commands_new/kis_image_set_resolution_command.h"
#include "commands_new/kis_activate_selection_mask_command.h"
#include "kis_composite_progress_proxy.h"
#include "kis_layer_composition.h"
#include "kis_wrapped_rect.h"
#include "kis_crop_saved_extra_data.h"
#include "kis_layer_utils.h"

#include "kis_lod_transform.h"

#include "kis_suspend_projection_updates_stroke_strategy.h"
#include "kis_sync_lod_cache_stroke_strategy.h"

#include "kis_projection_updates_filter.h"

#include "kis_layer_projection_plane.h"

#include "kis_update_time_monitor.h"
#include "tiles3/kis_lockless_stack.h"

#include <QtCore>

#include <functional>

#include "kis_time_range.h"

#include "KisRunnableBasedStrokeStrategy.h"
#include "KisRunnableStrokeJobData.h"
#include "KisRunnableStrokeJobUtils.h"
#include "KisRunnableStrokeJobsInterface.h"

// #define SANITY_CHECKS

#ifdef SANITY_CHECKS
#define SANITY_CHECK_LOCKED(name)                                       \
    if (!locked()) warnKrita() << "Locking policy failed:" << name          \
                               << "has been called without the image"       \
                                  "being locked";
#else
#define SANITY_CHECK_LOCKED(name)
#endif


struct KisImageSPStaticRegistrar {
    KisImageSPStaticRegistrar() {
        qRegisterMetaType<KisImageSP>("KisImageSP");
    }
};
static KisImageSPStaticRegistrar __registrar;

class KisImage::KisImagePrivate
{
public:
    KisImagePrivate(KisImage *_q, qint32 w, qint32 h,
                    const KoColorSpace *c,
                    KisUndoStore *undo,
                    KisImageAnimationInterface *_animationInterface)
        : q(_q)
        , lockedForReadOnly(false)
        , width(w)
        , height(h)
        , colorSpace(c ? c : KoColorSpaceRegistry::instance()->rgb8())
        , nserver(1)
        , undoStore(undo ? undo : new KisDumbUndoStore())
        , legacyUndoAdapter(undoStore.data(), _q)
        , postExecutionUndoAdapter(undoStore.data(), _q)
        , signalRouter(_q)
        , animationInterface(_animationInterface)
        , scheduler(_q, _q)
        , axesCenter(QPointF(0.5, 0.5))
    {
        {
            KisImageConfig cfg(true);
            if (cfg.enableProgressReporting()) {
                scheduler.setProgressProxy(&compositeProgressProxy);
            }

            // Each of these lambdas defines a new factory function.
            scheduler.setLod0ToNStrokeStrategyFactory(
                [=](bool forgettable) {
                    return KisLodSyncPair(
                        new KisSyncLodCacheStrokeStrategy(KisImageWSP(q), forgettable),
                        KisSyncLodCacheStrokeStrategy::createJobsData(KisImageWSP(q)));
                });

            scheduler.setSuspendUpdatesStrokeStrategyFactory(
                [=]() {
                    return KisSuspendResumePair(
                        new KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP(q), true),
                        KisSuspendProjectionUpdatesStrokeStrategy::createSuspendJobsData(KisImageWSP(q)));
                });

            scheduler.setResumeUpdatesStrokeStrategyFactory(
                [=]() {
                    return KisSuspendResumePair(
                        new KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP(q), false),
                        KisSuspendProjectionUpdatesStrokeStrategy::createResumeJobsData(KisImageWSP(q)));
                });
        }

        connect(q, SIGNAL(sigImageModified()), KisMemoryStatisticsServer::instance(), SLOT(notifyImageChanged()));
    }

    ~KisImagePrivate() {
        /**
         * Stop animation interface. It may use the rootLayer.
         */
        delete animationInterface;

        /**
         * First delete the nodes, while strokes
         * and undo are still alive
         */
        rootLayer.clear();
    }

    KisImage *q;

    quint32 lockCount = 0;
    bool lockedForReadOnly;

    qint32 width;
    qint32 height;

    double xres = 1.0;
    double yres = 1.0;

    const KoColorSpace * colorSpace;
    KisProofingConfigurationSP proofingConfig;

    KisSelectionSP deselectedGlobalSelection;
    KisGroupLayerSP rootLayer; // The layers are contained in here
    KisSelectionMaskSP targetOverlaySelectionMask; // the overlay switching stroke will try to switch into this mask
    KisSelectionMaskSP overlaySelectionMask;
    QList<KisLayerCompositionSP> compositions;
    KisNodeSP isolatedRootNode;
    bool wrapAroundModePermitted = false;

    KisNameServer nserver;

    QScopedPointer<KisUndoStore> undoStore;
    KisLegacyUndoAdapter legacyUndoAdapter;
    KisPostExecutionUndoAdapter postExecutionUndoAdapter;

    vKisAnnotationSP annotations;

    QAtomicInt disableUIUpdateSignals;
    KisLocklessStack<QRect> savedDisabledUIUpdates;

    KisProjectionUpdatesFilterSP projectionUpdatesFilter;
    KisImageSignalRouter signalRouter;
    KisImageAnimationInterface *animationInterface;
    KisUpdateScheduler scheduler;
    QAtomicInt disableDirtyRequests;


    KisCompositeProgressProxy compositeProgressProxy;

    bool blockLevelOfDetail = false;

    QPointF axesCenter;
    bool allowMasksOnRootNode = false;

    bool tryCancelCurrentStrokeAsync();

    void notifyProjectionUpdatedInPatches(const QRect &rc, QVector<KisRunnableStrokeJobData *> &jobs);
};

KisImage::KisImage(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace * colorSpace, const QString& name)
        : QObject(0)
        , KisShared()
        , m_d(new KisImagePrivate(this, width, height,
                                  colorSpace, undoStore,
                                  new KisImageAnimationInterface(this)))
{
    // make sure KisImage belongs to the GUI thread
    moveToThread(qApp->thread());
    connect(this, SIGNAL(sigInternalStopIsolatedModeRequested()), SLOT(stopIsolatedMode()));

    setObjectName(name);
    setRootLayer(new KisGroupLayer(this, "root", OPACITY_OPAQUE_U8));
}

KisImage::~KisImage()
{
    dbgImage << "deleting kisimage" << objectName();

    /**
     * Request the tools to end currently running strokes
     */
    waitForDone();

    delete m_d;
    disconnect(); // in case Qt gets confused
}

KisImage *KisImage::clone(bool exactCopy)
{
    return new KisImage(*this, 0, exactCopy);
}

KisImage::KisImage(const KisImage& rhs, KisUndoStore *undoStore, bool exactCopy)
    : KisNodeFacade(),
      KisNodeGraphListener(),
      KisShared(),
      m_d(new KisImagePrivate(this,
                              rhs.width(), rhs.height(),
                              rhs.colorSpace(),
                              undoStore ? undoStore : new KisDumbUndoStore(),
                              new KisImageAnimationInterface(*rhs.animationInterface(), this)))
{
    // make sure KisImage belongs to the GUI thread
    moveToThread(qApp->thread());
    connect(this, SIGNAL(sigInternalStopIsolatedModeRequested()), SLOT(stopIsolatedMode()));

    setObjectName(rhs.objectName());

    m_d->xres = rhs.m_d->xres;
    m_d->yres = rhs.m_d->yres;

    m_d->allowMasksOnRootNode = rhs.m_d->allowMasksOnRootNode;

    if (rhs.m_d->proofingConfig) {
        m_d->proofingConfig = toQShared(new KisProofingConfiguration(*rhs.m_d->proofingConfig));
    }

    KisNodeSP newRoot = rhs.root()->clone();
    newRoot->setGraphListener(this);
    newRoot->setImage(this);
    m_d->rootLayer = dynamic_cast<KisGroupLayer*>(newRoot.data());
    setRoot(newRoot);

    if (exactCopy || rhs.m_d->isolatedRootNode) {
        QQueue<KisNodeSP> linearizedNodes;
        KisLayerUtils::recursiveApplyNodes(rhs.root(),
            [&linearizedNodes](KisNodeSP node) {
                linearizedNodes.enqueue(node);
            });
        KisLayerUtils::recursiveApplyNodes(newRoot,
            [&linearizedNodes, exactCopy, &rhs, this](KisNodeSP node) {
                KisNodeSP refNode = linearizedNodes.dequeue();

                if (exactCopy) {
                    node->setUuid(refNode->uuid());
                }

                if (rhs.m_d->isolatedRootNode &&
                    rhs.m_d->isolatedRootNode == refNode) {

                    m_d->isolatedRootNode = node;
                }
            });
    }

    Q_FOREACH (KisLayerCompositionSP comp, rhs.m_d->compositions) {
        m_d->compositions << toQShared(new KisLayerComposition(*comp, this));
    }

    rhs.m_d->nserver = KisNameServer(rhs.m_d->nserver);

    vKisAnnotationSP newAnnotations;
    Q_FOREACH (KisAnnotationSP annotation, rhs.m_d->annotations) {
        newAnnotations << annotation->clone();
    }
    m_d->annotations = newAnnotations;

    KIS_ASSERT_RECOVER_NOOP(!rhs.m_d->projectionUpdatesFilter);
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_d->disableUIUpdateSignals);
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_d->disableDirtyRequests);

    m_d->blockLevelOfDetail = rhs.m_d->blockLevelOfDetail;

    /**
     * The overlay device is not inherited when cloning the image!
     */
    if (rhs.m_d->overlaySelectionMask) {
        const QRect dirtyRect = rhs.m_d->overlaySelectionMask->extent();
        m_d->rootLayer->setDirty(dirtyRect);
    }
}

void KisImage::aboutToAddANode(KisNode *parent, int index)
{
    KisNodeGraphListener::aboutToAddANode(parent, index);
    SANITY_CHECK_LOCKED("aboutToAddANode");
}

void KisImage::nodeHasBeenAdded(KisNode *parent, int index)
{
    KisNodeGraphListener::nodeHasBeenAdded(parent, index);

    SANITY_CHECK_LOCKED("nodeHasBeenAdded");
    m_d->signalRouter.emitNodeHasBeenAdded(parent, index);
}

void KisImage::aboutToRemoveANode(KisNode *parent, int index)
{
    KisNodeSP deletedNode = parent->at(index);
    if (!dynamic_cast<KisSelectionMask*>(deletedNode.data()) &&
        deletedNode == m_d->isolatedRootNode) {

        emit sigInternalStopIsolatedModeRequested();
    }

    KisNodeGraphListener::aboutToRemoveANode(parent, index);

    SANITY_CHECK_LOCKED("aboutToRemoveANode");
    m_d->signalRouter.emitAboutToRemoveANode(parent, index);
}

void KisImage::nodeChanged(KisNode* node)
{
    KisNodeGraphListener::nodeChanged(node);
    requestStrokeEnd();
    m_d->signalRouter.emitNodeChanged(node);
}

void KisImage::invalidateAllFrames()
{
    invalidateFrames(KisTimeRange::infinite(0), QRect());
}

void KisImage::setOverlaySelectionMask(KisSelectionMaskSP mask)
{
    if (m_d->targetOverlaySelectionMask == mask) return;

    m_d->targetOverlaySelectionMask = mask;

    struct UpdateOverlaySelectionStroke : public KisSimpleStrokeStrategy {
        UpdateOverlaySelectionStroke(KisImageSP image)
            : KisSimpleStrokeStrategy("update-overlay-selection-mask", kundo2_noi18n("update-overlay-selection-mask")),
              m_image(image)
        {
            this->enableJob(JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
            setClearsRedoOnStart(false);
        }

        void initStrokeCallback() {
            KisSelectionMaskSP oldMask = m_image->m_d->overlaySelectionMask;
            KisSelectionMaskSP newMask = m_image->m_d->targetOverlaySelectionMask;
            if (oldMask == newMask) return;

            KIS_SAFE_ASSERT_RECOVER_RETURN(!newMask || newMask->graphListener() == m_image);

            m_image->m_d->overlaySelectionMask = newMask;

            if (oldMask || newMask) {
                m_image->m_d->rootLayer->notifyChildMaskChanged();
            }

            if (oldMask) {
                m_image->m_d->rootLayer->setDirtyDontResetAnimationCache(oldMask->extent());
            }

            if (newMask) {
                newMask->setDirty();
            }

            m_image->undoAdapter()->emitSelectionChanged();
        }

    private:
        KisImageSP m_image;
    };

    KisStrokeId id = startStroke(new UpdateOverlaySelectionStroke(this));
    endStroke(id);
}

KisSelectionMaskSP KisImage::overlaySelectionMask() const
{
    return m_d->overlaySelectionMask;
}

bool KisImage::hasOverlaySelectionMask() const
{
    return m_d->overlaySelectionMask;
}

KisSelectionSP KisImage::globalSelection() const
{
    KisSelectionMaskSP selectionMask = m_d->rootLayer->selectionMask();
    if (selectionMask) {
        return selectionMask->selection();
    } else {
        return 0;
    }
}

void KisImage::setGlobalSelection(KisSelectionSP globalSelection)
{
    KisSelectionMaskSP selectionMask = m_d->rootLayer->selectionMask();

    if (!globalSelection) {
        if (selectionMask) {
            removeNode(selectionMask);
        }
    }
    else {
        if (!selectionMask) {
            selectionMask = new KisSelectionMask(this);
            selectionMask->initSelection(m_d->rootLayer);
            addNode(selectionMask);
            // If we do not set the selection now, the setActive call coming next
            // can be very, very expensive, depending on the size of the image.
            selectionMask->setSelection(globalSelection);
            selectionMask->setActive(true);
        }
        else {
            selectionMask->setSelection(globalSelection);
        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->rootLayer->childCount() > 0);
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->rootLayer->selectionMask());
    }

    m_d->deselectedGlobalSelection = 0;
    m_d->legacyUndoAdapter.emitSelectionChanged();
}

void KisImage::deselectGlobalSelection()
{
    KisSelectionSP savedSelection = globalSelection();
    setGlobalSelection(0);
    m_d->deselectedGlobalSelection = savedSelection;
}

bool KisImage::canReselectGlobalSelection()
{
    return m_d->deselectedGlobalSelection;
}

void KisImage::reselectGlobalSelection()
{
    if(m_d->deselectedGlobalSelection) {
        setGlobalSelection(m_d->deselectedGlobalSelection);
    }
}

QString KisImage::nextLayerName(const QString &_baseName) const
{
    QString baseName = _baseName;

    if (m_d->nserver.currentSeed() == 0) {
        m_d->nserver.number();
        return i18n("background");
    }

    if (baseName.isEmpty()) {
        baseName = i18n("Layer");
    }

    return QString("%1 %2").arg(baseName).arg(m_d->nserver.number());
}

void KisImage::rollBackLayerName()
{
    m_d->nserver.rollback();
}

KisCompositeProgressProxy* KisImage::compositeProgressProxy()
{
    return &m_d->compositeProgressProxy;
}

bool KisImage::locked() const
{
    return m_d->lockCount != 0;
}

void KisImage::barrierLock(bool readOnly)
{
    if (!locked()) {
        requestStrokeEnd();
        m_d->scheduler.barrierLock();
        m_d->lockedForReadOnly = readOnly;
    } else {
        m_d->lockedForReadOnly &= readOnly;
    }

    m_d->lockCount++;
}

bool KisImage::tryBarrierLock(bool readOnly)
{
    bool result = true;

    if (!locked()) {
        result = m_d->scheduler.tryBarrierLock();
        m_d->lockedForReadOnly = readOnly;
    }

    if (result) {
        m_d->lockCount++;
        m_d->lockedForReadOnly &= readOnly;
    }

    return result;
}

bool KisImage::isIdle(bool allowLocked)
{
    return (allowLocked || !locked()) && m_d->scheduler.isIdle();
}

void KisImage::lock()
{
    if (!locked()) {
        requestStrokeEnd();
        m_d->scheduler.lock();
    }
    m_d->lockCount++;
    m_d->lockedForReadOnly = false;
}

void KisImage::unlock()
{
    Q_ASSERT(locked());

    if (locked()) {
        m_d->lockCount--;

        if (m_d->lockCount == 0) {
            m_d->scheduler.unlock(!m_d->lockedForReadOnly);
        }
    }
}

void KisImage::blockUpdates()
{
    m_d->scheduler.blockUpdates();
}

void KisImage::unblockUpdates()
{
    m_d->scheduler.unblockUpdates();
}

void KisImage::setSize(const QSize& size)
{
    m_d->width = size.width();
    m_d->height = size.height();
}

void KisImage::resizeImageImpl(const QRect& newRect, bool cropLayers)
{
    if (newRect == bounds() && !cropLayers) return;

    KUndo2MagicString actionName = cropLayers ?
        kundo2_i18n("Crop Image") :
        kundo2_i18n("Resize Image");

    KisImageSignalVector emitSignals;
    emitSignals << ComplexSizeChangedSignal(newRect, newRect.size());
    emitSignals << ModifiedSignal;

    KisCropSavedExtraData *extraData =
        new KisCropSavedExtraData(cropLayers ?
                                  KisCropSavedExtraData::CROP_IMAGE :
                                  KisCropSavedExtraData::RESIZE_IMAGE,
                                  newRect);

    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       KisProcessingApplicator::RECURSIVE |
                                       KisProcessingApplicator::NO_UI_UPDATES,
                                       emitSignals, actionName, extraData);

    if (cropLayers || !newRect.topLeft().isNull()) {
        KisProcessingVisitorSP visitor =
            new KisCropProcessingVisitor(newRect, cropLayers, true);
        applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    }
    applicator.applyCommand(new KisImageResizeCommand(this, newRect.size()));
    applicator.end();
}

void KisImage::resizeImage(const QRect& newRect)
{
    resizeImageImpl(newRect, false);
}

void KisImage::cropImage(const QRect& newRect)
{
    resizeImageImpl(newRect, true);
}


void KisImage::cropNode(KisNodeSP node, const QRect& newRect)
{
    bool isLayer = qobject_cast<KisLayer*>(node.data());
    KUndo2MagicString actionName = isLayer ?
        kundo2_i18n("Crop Layer") :
        kundo2_i18n("Crop Mask");

    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisCropSavedExtraData *extraData =
        new KisCropSavedExtraData(KisCropSavedExtraData::CROP_LAYER,
                                  newRect, node);

    KisProcessingApplicator applicator(this, node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName, extraData);

    KisProcessingVisitorSP visitor =
        new KisCropProcessingVisitor(newRect, true, false);
    applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    applicator.end();
}

void KisImage::scaleImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy)
{
    bool resolutionChanged = xres != xRes() && yres != yRes();
    bool sizeChanged = size != this->size();

    if (!resolutionChanged && !sizeChanged) return;

    KisImageSignalVector emitSignals;
    if (resolutionChanged) emitSignals << ResolutionChangedSignal;
    if (sizeChanged) emitSignals << ComplexSizeChangedSignal(bounds(), size);
    emitSignals << ModifiedSignal;

    KUndo2MagicString actionName = sizeChanged ?
        kundo2_i18n("Scale Image") :
        kundo2_i18n("Change Image Resolution");

    KisProcessingApplicator::ProcessingFlags signalFlags =
        (resolutionChanged || sizeChanged) ?
                KisProcessingApplicator::NO_UI_UPDATES :
                KisProcessingApplicator::NONE;

    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       KisProcessingApplicator::RECURSIVE | signalFlags,
                                       emitSignals, actionName);

    qreal sx = qreal(size.width()) / this->size().width();
    qreal sy = qreal(size.height()) / this->size().height();

    QTransform shapesCorrection;

    if (resolutionChanged) {
        shapesCorrection = QTransform::fromScale(xRes() / xres, yRes() / yres);
    }

    KisProcessingVisitorSP visitor =
        new KisTransformProcessingVisitor(sx, sy,
                                          0, 0,
                                          QPointF(),
                                          0,
                                          0, 0,
                                          filterStrategy,
                                          shapesCorrection);

    applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);

    if (resolutionChanged) {
        KUndo2Command *parent =
            new KisResetShapesCommand(m_d->rootLayer);
        new KisImageSetResolutionCommand(this, xres, yres, parent);
        applicator.applyCommand(parent);
    }

    if (sizeChanged) {
        applicator.applyCommand(new KisImageResizeCommand(this, size));
    }

    applicator.end();
}

void KisImage::scaleNode(KisNodeSP node, const QPointF &center, qreal scaleX, qreal scaleY, KisFilterStrategy *filterStrategy, KisSelectionSP selection)
{
    KUndo2MagicString actionName(kundo2_i18n("Scale Layer"));
    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    QPointF offset;
    {
        KisTransformWorker worker(0,
                                  scaleX, scaleY,
                                  0, 0, 0, 0,
                                  0.0,
                                  0, 0, 0, 0);
        QTransform transform = worker.transform();

        offset = center - transform.map(center);
    }

    KisProcessingApplicator applicator(this, node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    KisTransformProcessingVisitor *visitor =
        new KisTransformProcessingVisitor(scaleX, scaleY,
                                          0, 0,
                                          QPointF(),
                                          0,
                                          offset.x(), offset.y(),
                                          filterStrategy);

    visitor->setSelection(selection);

    if (selection) {
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    } else {
        applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    }

    applicator.end();
}

void KisImage::rotateImpl(const KUndo2MagicString &actionName,
                          KisNodeSP rootNode,
                          double radians,
                          bool resizeImage,
                          KisSelectionSP selection)
{
    // we can either transform (and resize) the whole image or
    // transform a selection, we cannot do both at the same time
    KIS_SAFE_ASSERT_RECOVER(!(bool(selection) && resizeImage)) {
        selection = 0;
    }

    const QRect baseBounds =
        resizeImage ? bounds() :
        selection ? selection->selectedExactRect() :
        rootNode->exactBounds();

    QPointF offset;
    QSize newSize;

    {
        KisTransformWorker worker(0,
                                  1.0, 1.0,
                                  0, 0, 0, 0,
                                  radians,
                                  0, 0, 0, 0);
        QTransform transform = worker.transform();

        if (resizeImage) {
            QRect newRect = transform.mapRect(baseBounds);
            newSize = newRect.size();
            offset = -newRect.topLeft();
        }
        else {
            QPointF origin = QRectF(baseBounds).center();

            newSize = size();
            offset = -(transform.map(origin) - origin);
        }
    }

    bool sizeChanged = resizeImage &&
        (newSize.width() != baseBounds.width() ||
         newSize.height() != baseBounds.height());

    // These signals will be emitted after processing is done
    KisImageSignalVector emitSignals;
    if (sizeChanged) emitSignals << ComplexSizeChangedSignal(baseBounds, newSize);
    emitSignals << ModifiedSignal;

    // These flags determine whether updates are transferred to the UI during processing
    KisProcessingApplicator::ProcessingFlags signalFlags =
        sizeChanged ?
        KisProcessingApplicator::NO_UI_UPDATES :
        KisProcessingApplicator::NONE;


    KisProcessingApplicator applicator(this, rootNode,
                                       KisProcessingApplicator::RECURSIVE | signalFlags,
                                       emitSignals, actionName);

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Bicubic");

    KisTransformProcessingVisitor *visitor =
            new KisTransformProcessingVisitor(1.0, 1.0, 0.0, 0.0,
                                              QPointF(),
                                              radians,
                                              offset.x(), offset.y(),
                                              filter);
    if (selection) {
        visitor->setSelection(selection);
    }

    if (selection) {
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    } else {
        applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    }

    if (sizeChanged) {
        applicator.applyCommand(new KisImageResizeCommand(this, newSize));
    }
    applicator.end();
}


void KisImage::rotateImage(double radians)
{
    rotateImpl(kundo2_i18n("Rotate Image"), root(), radians, true, 0);
}

void KisImage::rotateNode(KisNodeSP node, double radians, KisSelectionSP selection)
{
    if (node->inherits("KisMask")) {
        rotateImpl(kundo2_i18n("Rotate Mask"), node, radians, false, selection);
    } else {
        rotateImpl(kundo2_i18n("Rotate Layer"), node, radians, false, selection);
    }
}

void KisImage::shearImpl(const KUndo2MagicString &actionName,
                         KisNodeSP rootNode,
                         bool resizeImage,
                         double angleX, double angleY,
                         KisSelectionSP selection)
{
    const QRect baseBounds =
        resizeImage ? bounds() :
        selection ? selection->selectedExactRect() :
        rootNode->exactBounds();

    const QPointF origin = QRectF(baseBounds).center();

    //angleX, angleY are in degrees
    const qreal pi = 3.1415926535897932385;
    const qreal deg2rad = pi / 180.0;

    qreal tanX = tan(angleX * deg2rad);
    qreal tanY = tan(angleY * deg2rad);

    QPointF offset;
    QSize newSize;

    {
        KisTransformWorker worker(0,
                                  1.0, 1.0,
                                  tanX, tanY, origin.x(), origin.y(),
                                  0,
                                  0, 0, 0, 0);

        QRect newRect = worker.transform().mapRect(baseBounds);
        newSize = newRect.size();
        if (resizeImage) offset = -newRect.topLeft();
    }

    if (newSize == baseBounds.size()) return;

    KisImageSignalVector emitSignals;
    if (resizeImage) emitSignals << ComplexSizeChangedSignal(baseBounds, newSize);
    emitSignals << ModifiedSignal;

    KisProcessingApplicator::ProcessingFlags signalFlags =
        KisProcessingApplicator::RECURSIVE;
    if (resizeImage) signalFlags |= KisProcessingApplicator::NO_UI_UPDATES;

    KisProcessingApplicator applicator(this, rootNode,
                                       signalFlags,
                                       emitSignals, actionName);

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Bilinear");

    KisTransformProcessingVisitor *visitor =
            new KisTransformProcessingVisitor(1.0, 1.0,
                                              tanX, tanY, origin,
                                              0,
                                              offset.x(), offset.y(),
                                              filter);

    if (selection) {
        visitor->setSelection(selection);
    }

    if (selection) {
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    } else {
        applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    }

    if (resizeImage) {
        applicator.applyCommand(new KisImageResizeCommand(this, newSize));
    }

    applicator.end();
}

void KisImage::shearNode(KisNodeSP node, double angleX, double angleY, KisSelectionSP selection)
{
    if (node->inherits("KisMask")) {
        shearImpl(kundo2_i18n("Shear Mask"), node, false,
                  angleX, angleY, selection);
    } else {
        shearImpl(kundo2_i18n("Shear Layer"), node, false,
                  angleX, angleY, selection);
    }
}

void KisImage::shear(double angleX, double angleY)
{
    shearImpl(kundo2_i18n("Shear Image"), m_d->rootLayer, true,
              angleX, angleY, 0);
}

void KisImage::convertImageColorSpace(const KoColorSpace *dstColorSpace,
                                      KoColorConversionTransformation::Intent renderingIntent,
                                      KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    if (!dstColorSpace) return;

    const KoColorSpace *srcColorSpace = m_d->colorSpace;

    undoAdapter()->beginMacro(kundo2_i18n("Convert Image Color Space"));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    undoAdapter()->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstColorSpace));

    KisColorSpaceConvertVisitor visitor(this, srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
    m_d->rootLayer->accept(visitor);

    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    undoAdapter()->endMacro();

    setModified();
}

bool KisImage::assignImageProfile(const KoColorProfile *profile)
{
    if (!profile) return false;

    const KoColorSpace *dstCs = KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    const KoColorSpace *srcCs = colorSpace();

    if (!dstCs) return false;

    m_d->colorSpace = dstCs;

    KisChangeProfileVisitor visitor(srcCs, dstCs);
    bool retval = m_d->rootLayer->accept(visitor);
    m_d->signalRouter.emitNotification(ProfileChangedSignal);
    return retval;

}

void KisImage::convertProjectionColorSpace(const KoColorSpace *dstColorSpace)
{
    if (*m_d->colorSpace == *dstColorSpace) return;

    undoAdapter()->beginMacro(kundo2_i18n("Convert Projection Color Space"));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), true));
    undoAdapter()->addCommand(new KisImageSetProjectionColorSpaceCommand(KisImageWSP(this), dstColorSpace));
    undoAdapter()->addCommand(new KisImageLockCommand(KisImageWSP(this), false));
    undoAdapter()->endMacro();

    setModified();
}

void KisImage::setProjectionColorSpace(const KoColorSpace * colorSpace)
{
    m_d->colorSpace = colorSpace;
    m_d->rootLayer->resetCache();
    m_d->signalRouter.emitNotification(ColorSpaceChangedSignal);
}

const KoColorSpace * KisImage::colorSpace() const
{
    return m_d->colorSpace;
}

const KoColorProfile * KisImage::profile() const
{
    return colorSpace()->profile();
}

double KisImage::xRes() const
{
    return m_d->xres;
}

double KisImage::yRes() const
{
    return m_d->yres;
}

void KisImage::setResolution(double xres, double yres)
{
    m_d->xres = xres;
    m_d->yres = yres;
    m_d->signalRouter.emitNotification(ResolutionChangedSignal);
}

QPointF KisImage::documentToPixel(const QPointF &documentCoord) const
{
    return QPointF(documentCoord.x() * xRes(), documentCoord.y() * yRes());
}

QPoint KisImage::documentToImagePixelFloored(const QPointF &documentCoord) const
{
    QPointF pixelCoord = documentToPixel(documentCoord);
    return QPoint(qFloor(pixelCoord.x()), qFloor(pixelCoord.y()));
}

QRectF KisImage::documentToPixel(const QRectF &documentRect) const
{
    return QRectF(documentToPixel(documentRect.topLeft()), documentToPixel(documentRect.bottomRight()));
}

QPointF KisImage::pixelToDocument(const QPointF &pixelCoord) const
{
    return QPointF(pixelCoord.x() / xRes(), pixelCoord.y() / yRes());
}

QPointF KisImage::pixelToDocument(const QPoint &pixelCoord) const
{
    return QPointF((pixelCoord.x() + 0.5) / xRes(), (pixelCoord.y() + 0.5) / yRes());
}

QRectF KisImage::pixelToDocument(const QRectF &pixelCoord) const
{
    return QRectF(pixelToDocument(pixelCoord.topLeft()), pixelToDocument(pixelCoord.bottomRight()));
}

qint32 KisImage::width() const
{
    return m_d->width;
}

qint32 KisImage::height() const
{
    return m_d->height;
}

KisGroupLayerSP KisImage::rootLayer() const
{
    Q_ASSERT(m_d->rootLayer);
    return m_d->rootLayer;
}

KisPaintDeviceSP KisImage::projection() const
{
    if (m_d->isolatedRootNode) {
        return m_d->isolatedRootNode->projection();
    }


    Q_ASSERT(m_d->rootLayer);
    KisPaintDeviceSP projection = m_d->rootLayer->projection();
    Q_ASSERT(projection);
    return projection;
}

qint32 KisImage::nlayers() const
{
    QStringList list;
    list << "KisLayer";

    KisCountVisitor visitor(list, KoProperties());
    m_d->rootLayer->accept(visitor);
    return visitor.count();
}

qint32 KisImage::nHiddenLayers() const
{
    QStringList list;
    list << "KisLayer";
    KoProperties properties;
    properties.setProperty("visible", false);
    KisCountVisitor visitor(list, properties);
    m_d->rootLayer->accept(visitor);

    return visitor.count();
}

void KisImage::flatten(KisNodeSP activeNode)
{
    KisLayerUtils::flattenImage(this, activeNode);
}

void KisImage::mergeMultipleLayers(QList<KisNodeSP> mergedNodes, KisNodeSP putAfter)
{
    if (!KisLayerUtils::tryMergeSelectionMasks(this, mergedNodes, putAfter)) {
        KisLayerUtils::mergeMultipleLayers(this, mergedNodes, putAfter);
    }
}

void KisImage::mergeDown(KisLayerSP layer, const KisMetaData::MergeStrategy* strategy)
{
    KisLayerUtils::mergeDown(this, layer, strategy);
}

void KisImage::flattenLayer(KisLayerSP layer)
{
    KisLayerUtils::flattenLayer(this, layer);
}


void KisImage::setModified()
{
    m_d->signalRouter.emitNotification(ModifiedSignal);
}

QImage KisImage::convertToQImage(QRect imageRect,
                                 const KoColorProfile * profile)
{
    qint32 x;
    qint32 y;
    qint32 w;
    qint32 h;
    imageRect.getRect(&x, &y, &w, &h);
    return convertToQImage(x, y, w, h, profile);
}

QImage KisImage::convertToQImage(qint32 x,
                                 qint32 y,
                                 qint32 w,
                                 qint32 h,
                                 const KoColorProfile * profile)
{
    KisPaintDeviceSP dev = projection();
    if (!dev) return QImage();
    QImage image = dev->convertToQImage(const_cast<KoColorProfile*>(profile), x, y, w, h,
                                        KoColorConversionTransformation::internalRenderingIntent(),
                                        KoColorConversionTransformation::internalConversionFlags());

    return image;
}



QImage KisImage::convertToQImage(const QSize& scaledImageSize, const KoColorProfile *profile)
{
    if (scaledImageSize.isEmpty()) {
        return QImage();
    }

    KisPaintDeviceSP dev = new KisPaintDevice(colorSpace());
    KisPainter gc;
    gc.copyAreaOptimized(QPoint(0, 0), projection(), dev, bounds());
    gc.end();
    double scaleX = qreal(scaledImageSize.width()) / width();
    double scaleY = qreal(scaledImageSize.height()) / height();

    QPointer<KoUpdater> updater = new KoDummyUpdater();

    KisTransformWorker worker(dev, scaleX, scaleY, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, updater, KisFilterStrategyRegistry::instance()->value("Bicubic"));
    worker.run();

    delete updater;

    return dev->convertToQImage(profile);
}
void KisImage::notifyLayersChanged()
{
    m_d->signalRouter.emitNotification(LayersChangedSignal);
}

QRect KisImage::bounds() const
{
    return QRect(0, 0, width(), height());
}

QRect KisImage::effectiveLodBounds() const
{
    QRect boundRect = bounds();

    const int lod = currentLevelOfDetail();
    if (lod > 0) {
        KisLodTransform t(lod);
        boundRect = t.map(boundRect);
    }

    return boundRect;
}

KisPostExecutionUndoAdapter* KisImage::postExecutionUndoAdapter() const
{
    const int lod = currentLevelOfDetail();
    return lod > 0 ?
        m_d->scheduler.lodNPostExecutionUndoAdapter() :
        &m_d->postExecutionUndoAdapter;
}

const KUndo2Command* KisImage::lastExecutedCommand() const
{
    return m_d->undoStore->presentCommand();
}

void KisImage::setUndoStore(KisUndoStore *undoStore)
{

    m_d->legacyUndoAdapter.setUndoStore(undoStore);
    m_d->postExecutionUndoAdapter.setUndoStore(undoStore);
    m_d->undoStore.reset(undoStore);
}

KisUndoStore* KisImage::undoStore()
{
    return m_d->undoStore.data();
}

KisUndoAdapter* KisImage::undoAdapter() const
{
    return &m_d->legacyUndoAdapter;
}

void KisImage::setDefaultProjectionColor(const KoColor &color)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->rootLayer);
    m_d->rootLayer->setDefaultProjectionColor(color);
}

KoColor KisImage::defaultProjectionColor() const
{
    KIS_ASSERT_RECOVER(m_d->rootLayer) {
        return KoColor(Qt::transparent, m_d->colorSpace);
    }

    return m_d->rootLayer->defaultProjectionColor();
}

void KisImage::setRootLayer(KisGroupLayerSP rootLayer)
{
    emit sigInternalStopIsolatedModeRequested();

    KoColor defaultProjectionColor(Qt::transparent, m_d->colorSpace);

    if (m_d->rootLayer) {
        m_d->rootLayer->setGraphListener(0);
        m_d->rootLayer->disconnect();

        KisPaintDeviceSP original = m_d->rootLayer->original();
        defaultProjectionColor = original->defaultPixel();
    }

    m_d->rootLayer = rootLayer;
    m_d->rootLayer->disconnect();
    m_d->rootLayer->setGraphListener(this);
    m_d->rootLayer->setImage(this);

    setRoot(m_d->rootLayer.data());
    this->setDefaultProjectionColor(defaultProjectionColor);
}

void KisImage::addAnnotation(KisAnnotationSP annotation)
{
    // Find the icc annotation, if there is one
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == annotation->type()) {
            *it = annotation;
            return;
        }
        ++it;
    }
    m_d->annotations.push_back(annotation);
}

KisAnnotationSP KisImage::annotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == type) {
            return *it;
        }
        ++it;
    }
    return KisAnnotationSP(0);
}

void KisImage::removeAnnotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it)->type() == type) {
            m_d->annotations.erase(it);
            return;
        }
        ++it;
    }
}

vKisAnnotationSP_it KisImage::beginAnnotations()
{
    return m_d->annotations.begin();
}

vKisAnnotationSP_it KisImage::endAnnotations()
{
    return m_d->annotations.end();
}

void KisImage::notifyAboutToBeDeleted()
{
    emit sigAboutToBeDeleted();
}

KisImageSignalRouter* KisImage::signalRouter()
{
    return &m_d->signalRouter;
}

void KisImage::waitForDone()
{
    requestStrokeEnd();
    m_d->scheduler.waitForDone();
}

KisStrokeId KisImage::startStroke(KisStrokeStrategy *strokeStrategy)
{
    /**
     * Ask open strokes to end gracefully. All the strokes clients
     * (including the one calling this method right now) will get
     * a notification that they should probably end their strokes.
     * However this is purely their choice whether to end a stroke
     * or not.
     */
    if (strokeStrategy->requestsOtherStrokesToEnd()) {
        requestStrokeEnd();
    }

    /**
     * Some of the strokes can cancel their work with undoing all the
     * changes they did to the paint devices. The problem is that undo
     * stack will know nothing about it. Therefore, just notify it
     * explicitly
     */
    if (strokeStrategy->clearsRedoOnStart()) {
        m_d->undoStore->purgeRedoState();
    }

    return m_d->scheduler.startStroke(strokeStrategy);
}

void KisImage::KisImagePrivate::notifyProjectionUpdatedInPatches(const QRect &rc, QVector<KisRunnableStrokeJobData*> &jobs)
{
    KisImageConfig imageConfig(true);
    int patchWidth = imageConfig.updatePatchWidth();
    int patchHeight = imageConfig.updatePatchHeight();

    for (int y = 0; y < rc.height(); y += patchHeight) {
        for (int x = 0; x < rc.width(); x += patchWidth) {
            QRect patchRect(x, y, patchWidth, patchHeight);
            patchRect &= rc;

            KritaUtils::addJobConcurrent(jobs, std::bind(&KisImage::notifyProjectionUpdated, q, patchRect));
        }
    }
}

bool KisImage::startIsolatedMode(KisNodeSP node)
{
    struct StartIsolatedModeStroke : public KisRunnableBasedStrokeStrategy {
        StartIsolatedModeStroke(KisNodeSP node, KisImageSP image)
            : KisRunnableBasedStrokeStrategy("start-isolated-mode", kundo2_noi18n("start-isolated-mode")),
              m_node(node),
              m_image(image)
        {
            this->enableJob(JOB_INIT, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
            this->enableJob(JOB_DOSTROKE, true);
            setClearsRedoOnStart(false);
        }

        void initStrokeCallback() {
            // pass-though node don't have any projection prepared, so we should
            // explicitly regenerate it before activating isolated mode.
            m_node->projectionLeaf()->explicitlyRegeneratePassThroughProjection();

            m_image->m_d->isolatedRootNode = m_node;
            emit m_image->sigIsolatedModeChanged();

            // the GUI uses our thread to do the color space conversion so we
            // need to emit this signal in multiple threads
            QVector<KisRunnableStrokeJobData*> jobs;
            m_image->m_d->notifyProjectionUpdatedInPatches(m_image->bounds(), jobs);
            this->runnableJobsInterface()->addRunnableJobs(jobs);


            m_image->invalidateAllFrames();
        }

    private:
        KisNodeSP m_node;
        KisImageSP m_image;
    };

    KisStrokeId id = startStroke(new StartIsolatedModeStroke(node, this));
    endStroke(id);

    return true;
}

void KisImage::stopIsolatedMode()
{
    if (!m_d->isolatedRootNode)  return;

    struct StopIsolatedModeStroke : public KisRunnableBasedStrokeStrategy {
        StopIsolatedModeStroke(KisImageSP image)
            : KisRunnableBasedStrokeStrategy("stop-isolated-mode", kundo2_noi18n("stop-isolated-mode")),
              m_image(image)
        {
            this->enableJob(JOB_INIT);
            this->enableJob(JOB_DOSTROKE, true);
            setClearsRedoOnStart(false);
        }

        void initStrokeCallback() {
            if (!m_image->m_d->isolatedRootNode)  return;

            //KisNodeSP oldRootNode = m_image->m_d->isolatedRootNode;
            m_image->m_d->isolatedRootNode = 0;

            emit m_image->sigIsolatedModeChanged();

            m_image->invalidateAllFrames();

            // the GUI uses our thread to do the color space conversion so we
            // need to emit this signal in multiple threads
            QVector<KisRunnableStrokeJobData*> jobs;
            m_image->m_d->notifyProjectionUpdatedInPatches(m_image->bounds(), jobs);
            this->runnableJobsInterface()->addRunnableJobs(jobs);

            // TODO: Substitute notifyProjectionUpdated() with this code
            // when update optimization is implemented
            //
            // QRect updateRect = bounds() | oldRootNode->extent();
            // oldRootNode->setDirty(updateRect);
        }

    private:
        KisImageSP m_image;
    };

    KisStrokeId id = startStroke(new StopIsolatedModeStroke(this));
    endStroke(id);
}

KisNodeSP KisImage::isolatedModeRoot() const
{
    return m_d->isolatedRootNode;
}

void KisImage::addJob(KisStrokeId id, KisStrokeJobData *data)
{
    KisUpdateTimeMonitor::instance()->reportJobStarted(data);
    m_d->scheduler.addJob(id, data);
}

void KisImage::endStroke(KisStrokeId id)
{
    m_d->scheduler.endStroke(id);
}

bool KisImage::cancelStroke(KisStrokeId id)
{
    return m_d->scheduler.cancelStroke(id);
}

bool KisImage::KisImagePrivate::tryCancelCurrentStrokeAsync()
{
    return scheduler.tryCancelCurrentStrokeAsync();
}

void KisImage::requestUndoDuringStroke()
{
    emit sigUndoDuringStrokeRequested();
}

void KisImage::requestStrokeCancellation()
{
    if (!m_d->tryCancelCurrentStrokeAsync()) {
        emit sigStrokeCancellationRequested();
    }
}

UndoResult KisImage::tryUndoUnfinishedLod0Stroke()
{
    return m_d->scheduler.tryUndoLastStrokeAsync();
}

void KisImage::requestStrokeEnd()
{
    emit sigStrokeEndRequested();
    emit sigStrokeEndRequestedActiveNodeFiltered();
}

void KisImage::requestStrokeEndActiveNode()
{
    emit sigStrokeEndRequested();
}

void KisImage::refreshGraph(KisNodeSP root)
{
    refreshGraph(root, bounds(), bounds());
}

void KisImage::refreshGraph(KisNodeSP root, const QRect &rc, const QRect &cropRect)
{
    if (!root) root = m_d->rootLayer;

    m_d->animationInterface->notifyNodeChanged(root.data(), rc, true);
    m_d->scheduler.fullRefresh(root, rc, cropRect);
}

void KisImage::initialRefreshGraph()
{
    /**
     * NOTE: Tricky part. We set crop rect to null, so the clones
     * will not rely on precalculated projections of their sources
     */

    refreshGraphAsync(0, bounds(), QRect());
    waitForDone();
}

void KisImage::refreshGraphAsync(KisNodeSP root)
{
    refreshGraphAsync(root, bounds(), bounds());
}

void KisImage::refreshGraphAsync(KisNodeSP root, const QRect &rc)
{
    refreshGraphAsync(root, rc, bounds());
}

void KisImage::refreshGraphAsync(KisNodeSP root, const QRect &rc, const QRect &cropRect)
{
    if (!root) root = m_d->rootLayer;

    m_d->animationInterface->notifyNodeChanged(root.data(), rc, true);
    m_d->scheduler.fullRefreshAsync(root, rc, cropRect);
}

void KisImage::requestProjectionUpdateNoFilthy(KisNodeSP pseudoFilthy, const QRect &rc, const QRect &cropRect)
{
    KIS_ASSERT_RECOVER_RETURN(pseudoFilthy);

    m_d->animationInterface->notifyNodeChanged(pseudoFilthy.data(), rc, false);
    m_d->scheduler.updateProjectionNoFilthy(pseudoFilthy, rc, cropRect);
}

void KisImage::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
    m_d->scheduler.addSpontaneousJob(spontaneousJob);
}

bool KisImage::hasUpdatesRunning() const
{
    return m_d->scheduler.hasUpdatesRunning();
}

void KisImage::setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP filter)
{
    // update filters are *not* recursive!
    KIS_ASSERT_RECOVER_NOOP(!filter || !m_d->projectionUpdatesFilter);

    m_d->projectionUpdatesFilter = filter;
}

KisProjectionUpdatesFilterSP KisImage::projectionUpdatesFilter() const
{
    return m_d->projectionUpdatesFilter;
}

void KisImage::disableDirtyRequests()
{
    setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP(new KisDropAllProjectionUpdatesFilter()));
}

void KisImage::enableDirtyRequests()
{
    setProjectionUpdatesFilter(KisProjectionUpdatesFilterSP());
}

void KisImage::disableUIUpdates()
{
    m_d->disableUIUpdateSignals.ref();
}

void KisImage::notifyBatchUpdateStarted()
{
    m_d->signalRouter.emitNotifyBatchUpdateStarted();
}

void KisImage::notifyBatchUpdateEnded()
{
    m_d->signalRouter.emitNotifyBatchUpdateEnded();
}

void KisImage::notifyUIUpdateCompleted(const QRect &rc)
{
    notifyProjectionUpdated(rc);
}

QVector<QRect> KisImage::enableUIUpdates()
{
    m_d->disableUIUpdateSignals.deref();

    QRect rect;
    QVector<QRect> postponedUpdates;

    while (m_d->savedDisabledUIUpdates.pop(rect)) {
        postponedUpdates.append(rect);
    }

    return postponedUpdates;
}

void KisImage::notifyProjectionUpdated(const QRect &rc)
{
    KisUpdateTimeMonitor::instance()->reportUpdateFinished(rc);

    if (!m_d->disableUIUpdateSignals) {
        int lod = currentLevelOfDetail();
        QRect dirtyRect = !lod ? rc : KisLodTransform::upscaledRect(rc, lod);

        if (dirtyRect.isEmpty()) return;

        emit sigImageUpdated(dirtyRect);
    } else {
        m_d->savedDisabledUIUpdates.push(rc);
    }
}

void KisImage::setWorkingThreadsLimit(int value)
{
    m_d->scheduler.setThreadsLimit(value);
}

int KisImage::workingThreadsLimit() const
{
    return m_d->scheduler.threadsLimit();
}

void KisImage::notifySelectionChanged()
{
    /**
     * The selection is calculated asynchromously, so it is not
     * handled by disableUIUpdates() and other special signals of
     * KisImageSignalRouter
     */
    m_d->legacyUndoAdapter.emitSelectionChanged();

    /**
     * Editing of selection masks doesn't necessary produce a
     * setDirty() call, so in the end of the stroke we need to request
     * direct update of the UI's cache.
     */
    if (m_d->isolatedRootNode &&
        dynamic_cast<KisSelectionMask*>(m_d->isolatedRootNode.data())) {

        notifyProjectionUpdated(bounds());
    }
}

void KisImage::requestProjectionUpdateImpl(KisNode *node,
                                           const QVector<QRect> &rects,
                                           const QRect &cropRect)
{
    if (rects.isEmpty()) return;

    m_d->scheduler.updateProjection(node, rects, cropRect);
}

void KisImage::requestProjectionUpdate(KisNode *node, const QVector<QRect> &rects, bool resetAnimationCache)
{
    if (m_d->projectionUpdatesFilter
        && m_d->projectionUpdatesFilter->filter(this, node, rects, resetAnimationCache)) {

        return;
    }

    if (resetAnimationCache) {
        m_d->animationInterface->notifyNodeChanged(node, rects, false);
    }

    /**
     * Here we use 'permitted' instead of 'active' intentively,
     * because the updates may come after the actual stroke has been
     * finished. And having some more updates for the stroke not
     * supporting the wrap-around mode will not make much harm.
     */
    if (m_d->wrapAroundModePermitted) {
        QVector<QRect> allSplitRects;

        const QRect boundRect = effectiveLodBounds();
        Q_FOREACH (const QRect &rc, rects) {
            KisWrappedRect splitRect(rc, boundRect);
            allSplitRects.append(splitRect);
        }

        requestProjectionUpdateImpl(node, allSplitRects, boundRect);

    } else {
        requestProjectionUpdateImpl(node, rects, bounds());
    }

    KisNodeGraphListener::requestProjectionUpdate(node, rects, resetAnimationCache);
}

void KisImage::invalidateFrames(const KisTimeRange &range, const QRect &rect)
{
    m_d->animationInterface->invalidateFrames(range, rect);
}

void KisImage::requestTimeSwitch(int time)
{
    m_d->animationInterface->requestTimeSwitchNonGUI(time);
}

KisNode *KisImage::graphOverlayNode() const
{
    return m_d->overlaySelectionMask.data();
}

QList<KisLayerCompositionSP> KisImage::compositions()
{
    return m_d->compositions;
}

void KisImage::addComposition(KisLayerCompositionSP composition)
{
    m_d->compositions.append(composition);
}

void KisImage::removeComposition(KisLayerCompositionSP composition)
{
    m_d->compositions.removeAll(composition);
}

bool checkMasksNeedConversion(KisNodeSP root, const QRect &bounds)
{
    KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(root.data());
    if (mask &&
        (!bounds.contains(mask->paintDevice()->exactBounds()) ||
         mask->selection()->hasShapeSelection())) {

        return true;
    }

    KisNodeSP node = root->firstChild();

    while (node) {
        if (checkMasksNeedConversion(node, bounds)) {
            return true;
        }

        node = node->nextSibling();
    }

    return false;
}

void KisImage::setWrapAroundModePermitted(bool value)
{
    if (m_d->wrapAroundModePermitted != value) {
        requestStrokeEnd();
    }

    m_d->wrapAroundModePermitted = value;

    if (m_d->wrapAroundModePermitted &&
        checkMasksNeedConversion(root(), bounds())) {

        KisProcessingApplicator applicator(this, root(),
                                           KisProcessingApplicator::RECURSIVE,
                                           KisImageSignalVector() << ModifiedSignal,
                                           kundo2_i18n("Crop Selections"));

        KisProcessingVisitorSP visitor =
            new KisCropSelectionsProcessingVisitor(bounds());

        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
        applicator.end();
    }
}

bool KisImage::wrapAroundModePermitted() const
{
    return m_d->wrapAroundModePermitted;
}

bool KisImage::wrapAroundModeActive() const
{
    return m_d->wrapAroundModePermitted &&
        m_d->scheduler.wrapAroundModeSupported();
}

void KisImage::setDesiredLevelOfDetail(int lod)
{
    if (m_d->blockLevelOfDetail) {
        qWarning() << "WARNING: KisImage::setDesiredLevelOfDetail()"
                   << "was called while LoD functionality was being blocked!";
        return;
    }

    m_d->scheduler.setDesiredLevelOfDetail(lod);
}

int KisImage::currentLevelOfDetail() const
{
    if (m_d->blockLevelOfDetail) {
        return 0;
    }

    return m_d->scheduler.currentLevelOfDetail();
}

void KisImage::setLevelOfDetailBlocked(bool value)
{
    KisImageBarrierLockerRaw l(this);

    if (value && !m_d->blockLevelOfDetail) {
        m_d->scheduler.setDesiredLevelOfDetail(0);
    }

    m_d->blockLevelOfDetail = value;
}

void KisImage::explicitRegenerateLevelOfDetail()
{
    if (!m_d->blockLevelOfDetail) {
        m_d->scheduler.explicitRegenerateLevelOfDetail();
    }
}

bool KisImage::levelOfDetailBlocked() const
{
    return m_d->blockLevelOfDetail;
}

void KisImage::notifyNodeCollpasedChanged()
{
    emit sigNodeCollapsedChanged();
}

KisImageAnimationInterface* KisImage::animationInterface() const
{
    return m_d->animationInterface;
}

void KisImage::setProofingConfiguration(KisProofingConfigurationSP proofingConfig)
{
    m_d->proofingConfig = proofingConfig;
    emit sigProofingConfigChanged();
}

KisProofingConfigurationSP KisImage::proofingConfiguration() const
{
    if (m_d->proofingConfig) {
        return m_d->proofingConfig;
    }
    return KisProofingConfigurationSP();
}

QPointF KisImage::mirrorAxesCenter() const
{
    return m_d->axesCenter;
}

void KisImage::setMirrorAxesCenter(const QPointF &value) const
{
    m_d->axesCenter = value;
}

void KisImage::setAllowMasksOnRootNode(bool value)
{
    m_d->allowMasksOnRootNode = value;
}

bool KisImage::allowMasksOnRootNode() const
{
    return m_d->allowMasksOnRootNode;
}
