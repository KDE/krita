/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

#include <klocalizedstring.h>

#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorProfile.h"
#include <KoCompositeOpRegistry.h>
#include "KisProofingConfiguration.h"

#include "kis_adjustment_layer.h"
#include "kis_annotation.h"
#include "kis_count_visitor.h"
#include "kis_filter_strategy.h"
#include "kis_group_layer.h"
#include "commands/kis_image_commands.h"
#include "kis_layer.h"
#include "kis_meta_data_merge_strategy_registry.h"
#include "kis_paint_layer.h"
#include "kis_projection_leaf.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_transaction.h"
#include "kis_meta_data_merge_strategy.h"
#include "kis_memory_statistics_server.h"
#include "kis_node.h"
#include "kis_types.h"

#include "kis_image_config.h"
#include "kis_update_scheduler.h"
#include "kis_image_signal_router.h"
#include "kis_image_animation_interface.h"
#include "kis_keyframe_channel.h"
#include "kis_stroke_strategy.h"
#include "kis_simple_stroke_strategy.h"

#include "kis_undo_stores.h"
#include "kis_legacy_undo_adapter.h"
#include "kis_post_execution_undo_adapter.h"

#include "kis_transform_worker.h"
#include "kis_processing_applicator.h"
#include "processing/kis_crop_processing_visitor.h"
#include "processing/kis_crop_selections_processing_visitor.h"
#include "processing/kis_transform_processing_visitor.h"
#include "processing/kis_convert_color_space_processing_visitor.h"
#include "processing/kis_assign_profile_processing_visitor.h"
#include "commands_new/kis_image_resize_command.h"
#include "commands_new/kis_image_set_resolution_command.h"
#include "commands_new/kis_activate_selection_mask_command.h"
#include "kis_composite_progress_proxy.h"
#include "kis_layer_composition.h"
#include "kis_wrapped_rect.h"
#include "kis_crop_saved_extra_data.h"
#include "kis_layer_utils.h"
#include "kis_keyframe_channel.h"

#include "kis_lod_transform.h"

#include "kis_suspend_projection_updates_stroke_strategy.h"
#include "kis_sync_lod_cache_stroke_strategy.h"

#include "kis_projection_updates_filter.h"

#include "kis_layer_projection_plane.h"

#include "kis_update_time_monitor.h"
#include "kis_lockless_stack.h"

#include <QtCore>

#include <functional>

#include "kis_time_span.h"

#include "KisRunnableBasedStrokeStrategy.h"
#include "KisRunnableStrokeJobData.h"
#include "KisRunnableStrokeJobUtils.h"
#include "KisRunnableStrokeJobsInterface.h"

#include "KisBusyWaitBroker.h"
#include <KisStaticInitializer.h>
#include "KisImageGlobalSelectionManagementInterface.h"


// #define SANITY_CHECKS

#ifdef SANITY_CHECKS
#define SANITY_CHECK_LOCKED(name)                                       \
    if (!locked()) warnKrita() << "Locking policy failed:" << name          \
                               << "has been called without the image"       \
                                  "being locked";
#else
#define SANITY_CHECK_LOCKED(name)
#endif

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<KisImageSP>("KisImageSP");
}

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
        , globalSelectionInterface(_q)
        , isolateLayer(false)
        , isolateGroup(false)
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

            scheduler.setSuspendResumeUpdatesStrokeStrategyFactory(
                [=]() {
                    KisSuspendProjectionUpdatesStrokeStrategy::SharedDataSP data = KisSuspendProjectionUpdatesStrokeStrategy::createSharedData();

                    KisSuspendResumePair suspend(new KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP(q), true, data),
                                                 KisSuspendProjectionUpdatesStrokeStrategy::createSuspendJobsData(KisImageWSP(q)));
                    KisSuspendResumePair resume(new KisSuspendProjectionUpdatesStrokeStrategy(KisImageWSP(q), false, data),
                                                KisSuspendProjectionUpdatesStrokeStrategy::createResumeJobsData(KisImageWSP(q)));

                    return std::make_pair(suspend, resume);
                });

            scheduler.setPurgeRedoStateCallback(
                        [this] () {
                undoStore->purgeRedoState();
            });

            scheduler.setPostSyncLod0GUIPlaneRequestForResumeCallback(
                        [this] () {

                KIS_SAFE_ASSERT_RECOVER_NOOP(q->currentLevelOfDetail() == 0);

                bool addedUIUpdateRequestSuccessfully = false;

                for (auto it = std::make_reverse_iterator(projectionUpdatesFilters.end());
                     it != std::make_reverse_iterator(projectionUpdatesFilters.begin());
                     ++it) {

                    KisSuspendProjectionUpdatesStrokeStrategy::SuspendUpdatesFilterInterface *iface =
                        dynamic_cast<KisSuspendProjectionUpdatesStrokeStrategy::SuspendUpdatesFilterInterface*>(it->data());

                    if (iface) {
                        iface->addExplicitUIUpdateRect(q->bounds());
                        addedUIUpdateRequestSuccessfully = true;
                        break;
                    }
                }

                KIS_SAFE_ASSERT_RECOVER_NOOP(addedUIUpdateRequestSuccessfully);
            });
        }

        connect(q, SIGNAL(sigImageModified()), KisMemoryStatisticsServer::instance(), SLOT(notifyImageChanged()));
        connect(undoStore.data(), SIGNAL(historyStateChanged()), &signalRouter, SLOT(emitImageModifiedNotification()));
    }

    ~KisImagePrivate() {
        /**
         * First delete the nodes, while strokes
         * and undo are still alive
         */

        KIS_SAFE_ASSERT_RECOVER_NOOP(rootLayer->graphListener() == q);
        KIS_SAFE_ASSERT_RECOVER_NOOP(rootLayer->image() == q);

        /**
         * Firstly we need to disconnect the nodes from the image,
         * because some of the nodes (e.g. KisGroupLayer) may
         * request the image back via defaultBouds() and/or
         * animationInterface()
         */
        if (rootLayer->image() == q) {
            rootLayer->setImage(0);
        }

        if (rootLayer->graphListener() == q) {
            rootLayer->setGraphListener(0);
        }

        rootLayer.clear();

        /**
         * Stop animation interface. It may use the rootLayer.
         */
        delete animationInterface;
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

    KisImageGlobalSelectionManagementInterface globalSelectionInterface;
    KisSelectionMaskSP deselectedGlobalSelectionMask;
    KisGroupLayerSP rootLayer; // The layers are contained in here
    KisSelectionMaskSP targetOverlaySelectionMask; // the overlay switching stroke will try to switch into this mask
    KisSelectionMaskSP overlaySelectionMask;
    QList<KisLayerCompositionSP> compositions;

    KisNodeSP isolationRootNode;
    bool isolateLayer;
    bool isolateGroup;

    bool wrapAroundModePermitted = false;
    WrapAroundAxis wrapAroundModeAxis = WRAPAROUND_BOTH;

    QScopedPointer<KisUndoStore> undoStore;
    KisLegacyUndoAdapter legacyUndoAdapter;
    KisPostExecutionUndoAdapter postExecutionUndoAdapter;

    vKisAnnotationSP annotations;

    QAtomicInt disableUIUpdateSignals;
    KisLocklessStack<QRect> savedDisabledUIUpdates;

    // filters are applied in a reversed way, from rbegin() to rend()
    QVector<KisProjectionUpdatesFilterSP> projectionUpdatesFilters;
    QStack<KisProjectionUpdatesFilterCookie> disabledUpdatesCookies;
    KisImageSignalRouter signalRouter;
    KisImageAnimationInterface *animationInterface;
    KisUpdateScheduler scheduler;
    QAtomicInt disableDirtyRequests;

    KisCompositeProgressProxy compositeProgressProxy;

    QPointF axesCenter;
    bool allowMasksOnRootNode = false;

    void requestProjectionUpdateImpl(KisNode *node,
                                     const QVector<QRect> &rects,
                                     const QRect &cropRect,
                                     KisProjectionUpdateFlags flags);

    bool tryCancelCurrentStrokeAsync();

    void notifyProjectionUpdatedInPatches(const QRect &rc, QVector<KisRunnableStrokeJobData *> &jobs);

    void convertImageColorSpaceImpl(const KoColorSpace *dstColorSpace,
                                    bool convertLayers,
                                    KoColorConversionTransformation::Intent renderingIntent,
                                    KoColorConversionTransformation::ConversionFlags conversionFlags);

    struct SetImageProjectionColorSpace;
};

KisImageGlobalSelectionManagementInterface::KisImageGlobalSelectionManagementInterface(KisImage *_q)
    : q(_q)
{
}

KisSelectionMaskSP KisImageGlobalSelectionManagementInterface::deselectedGlobalSelection() const
{
    return q->m_d->deselectedGlobalSelectionMask;
}

void KisImageGlobalSelectionManagementInterface::setDeselectedGlobalSelection(KisSelectionMaskSP selectionMask)
{
    q->m_d->deselectedGlobalSelectionMask = selectionMask;
}

KisImage::KisImage(KisUndoStore *undoStore, qint32 width, qint32 height, const KoColorSpace *colorSpace, const QString& name)
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
    /**
     * Request the tools to end currently running strokes
     */
    waitForDone();

    delete m_d;
    disconnect(); // in case Qt gets confused
}

KisImageSP KisImage::fromQImage(const QImage &image, KisUndoStore *undoStore)
{
    const KoColorSpace *colorSpace = 0;

    switch (image.format()) {
    case QImage::Format_Invalid:
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        colorSpace = KoColorSpaceRegistry::instance()->graya8();
        break;
    case QImage::Format_Indexed8:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        break;
    case QImage::Format_RGB16:
        colorSpace = KoColorSpaceRegistry::instance()->rgb16();
        break;
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_RGB666:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_RGB555:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_RGB888:
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        break;
    case QImage::Format_BGR30:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_RGB30:
    case QImage::Format_A2RGB30_Premultiplied:
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        break;
    case QImage::Format_Alpha8:
        colorSpace = KoColorSpaceRegistry::instance()->alpha8();
        break;
    case QImage::Format_Grayscale8:
        colorSpace = KoColorSpaceRegistry::instance()->graya8();
        break;
    case QImage::Format_Grayscale16:
        colorSpace = KoColorSpaceRegistry::instance()->graya16();
        break;
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
        colorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);
        break;
    default:
        colorSpace = 0;
    }

    KisImageSP img = new KisImage(undoStore, image.width(), image.height(), colorSpace, i18n("Imported Image"));
    KisPaintLayerSP layer = new KisPaintLayer(img, img->nextLayerName(), 255);
    layer->paintDevice()->convertFromQImage(image, 0, 0, 0);
    img->addNode(layer.data(), img->rootLayer().data());

    return img;
}

KisImage *KisImage::clone(bool exactCopy)
{
    return new KisImage(*this, 0, exactCopy);
}

void KisImage::copyFromImage(const KisImage &rhs)
{
    copyFromImageImpl(rhs, REPLACE);
}

void KisImage::copyFromImageImpl(const KisImage &rhs, int policy)
{
    // make sure we choose exactly one from REPLACE and CONSTRUCT
    KIS_ASSERT_RECOVER_RETURN(bool(policy & REPLACE) != bool(policy & CONSTRUCT));

    /**
     * We should replace the root before emitting any signals, because some of the layers
     * may be subscribed to sigSizeChanged() signal (e.g. KisSelectionBasedLayer). So the
     * old layers should be fully detached before we actually Q_EMIT this signal.
     *
     * We should also change all the dimensional properties of the image before setting
     * the image on the nodes, because some nodes may emit signals if something changes
     * internally (e.g. resolution).
     *
     * See bug 447599 for more details.
     */

    const bool sizeChanged = m_d->width != rhs.width() || m_d->height != rhs.height();
    const bool colorSpaceChanged = *m_d->colorSpace != *rhs.colorSpace();
    const bool resolutionChanged = m_d->xres != rhs.m_d->xres || m_d->yres != rhs.m_d->yres;

    if (sizeChanged) {
        m_d->width = rhs.width();
        m_d->height = rhs.height();
    }

    if (colorSpaceChanged) {
        m_d->colorSpace = rhs.colorSpace();
    }

    if (resolutionChanged) {
        m_d->xres = rhs.m_d->xres;
        m_d->yres = rhs.m_d->yres;
    }

    // from KisImage::KisImage(const KisImage &, KisUndoStore *, bool)
    setObjectName(rhs.objectName());

    KisNodeSP oldRoot = this->root();
    KisNodeSP newRoot = rhs.root()->clone();
    newRoot->setGraphListener(this);
    newRoot->setImage(this);

    m_d->rootLayer = dynamic_cast<KisGroupLayer*>(newRoot.data());
    setRoot(newRoot);

    if (oldRoot) {
        oldRoot->setImage(0);
        oldRoot->setGraphListener(0);
        oldRoot->disconnect();
    }

    // only when replacing do we need to Q_EMIT signals
#define EMIT_IF_NEEDED if (!(policy & REPLACE)) {} else emit

    if (sizeChanged) {
        EMIT_IF_NEEDED sigSizeChanged(QPointF(), QPointF());
    }
    if (colorSpaceChanged) {
        EMIT_IF_NEEDED sigColorSpaceChanged(m_d->colorSpace);
    }
    if (resolutionChanged) {
        EMIT_IF_NEEDED sigResolutionChanged(m_d->xres, m_d->yres);
    }

    m_d->allowMasksOnRootNode = rhs.m_d->allowMasksOnRootNode;

    if (rhs.m_d->proofingConfig) {
        KisProofingConfigurationSP proofingConfig(new KisProofingConfiguration(*rhs.m_d->proofingConfig));
        if (policy & REPLACE) {
            setProofingConfiguration(proofingConfig);
        } else {
            m_d->proofingConfig = proofingConfig;
        }
    }

    bool exactCopy = policy & EXACT_COPY;

    if (exactCopy || rhs.m_d->isolationRootNode || rhs.m_d->overlaySelectionMask) {
        m_d->isolateLayer = rhs.m_d->isolateLayer;
        m_d->isolateGroup = rhs.m_d->isolateGroup;

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

                                               if (rhs.m_d->isolationRootNode &&
                                                   rhs.m_d->isolationRootNode == refNode) {
                                                   m_d->isolationRootNode = node;
                                               }

                                               if (rhs.m_d->overlaySelectionMask &&
                                                   KisNodeSP(rhs.m_d->overlaySelectionMask) == refNode) {
                                                   m_d->targetOverlaySelectionMask = dynamic_cast<KisSelectionMask*>(node.data());
                                                   m_d->overlaySelectionMask = m_d->targetOverlaySelectionMask;
                                                   m_d->rootLayer->notifyChildMaskChanged();
                                               }


                                               // Re-establish DefaultBounds Instances for Existing Nodes
                                               // This is a workaround for copy-constructors failing to pass
                                               // proper DefaultBounds due to either lacking image data on construction
                                               // We should change the way "DefaultBounds" works to try to make it
                                               // safer for threading races.
                                               using KeyframeChannelContainer = QMap<QString, KisKeyframeChannel*>;
                                               KeyframeChannelContainer keyframeChannels = node->keyframeChannels();
                                               for (KeyframeChannelContainer::iterator i = keyframeChannels.begin();
                                                    i != keyframeChannels.end(); i++) {
                                                   keyframeChannels[i.key()]->setNode(node);
                                               }
                                           });
    }

    KisLayerUtils::recursiveApplyNodes(newRoot,
                                       [](KisNodeSP node) {
                                           dbgImage << "Node: " << (void *)node.data();
                                       });



    m_d->compositions.clear();

    Q_FOREACH (KisLayerCompositionSP comp, rhs.m_d->compositions) {
        m_d->compositions << toQShared(new KisLayerComposition(*comp, this));
    }

    EMIT_IF_NEEDED sigLayersChangedAsync();

    vKisAnnotationSP newAnnotations;
    Q_FOREACH (KisAnnotationSP annotation, rhs.m_d->annotations) {
        newAnnotations << annotation->clone();
    }
    m_d->annotations = newAnnotations;

    KIS_ASSERT_RECOVER_NOOP(rhs.m_d->projectionUpdatesFilters.isEmpty());
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_d->disableUIUpdateSignals);
    KIS_ASSERT_RECOVER_NOOP(!rhs.m_d->disableDirtyRequests);

#undef EMIT_IF_NEEDED
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

    copyFromImageImpl(rhs, CONSTRUCT | (exactCopy ? EXACT_COPY : 0));
}

void KisImage::aboutToAddANode(KisNode *parent, int index)
{
    KisNodeGraphListener::aboutToAddANode(parent, index);
    SANITY_CHECK_LOCKED("aboutToAddANode");
}

void KisImage::nodeHasBeenAdded(KisNode *parent, int index, KisNodeAdditionFlags flags)
{
    KisNodeGraphListener::nodeHasBeenAdded(parent, index, flags);

    KisLayerUtils::recursiveApplyNodes(KisSharedPtr<KisNode>(parent), [this](KisNodeSP node){
       QMap<QString, KisKeyframeChannel*> chans = node->keyframeChannels();
       Q_FOREACH(KisKeyframeChannel* chan, chans.values()) {
           chan->setNode(node);
           this->keyframeChannelHasBeenAdded(node.data(), chan);
       }
    });

    SANITY_CHECK_LOCKED("nodeHasBeenAdded");
    m_d->signalRouter.emitNodeHasBeenAdded(parent, index, flags);
}

void KisImage::aboutToRemoveANode(KisNode *parent, int index)
{
    KisNodeSP deletedNode = parent->at(index);
    if (!dynamic_cast<KisSelectionMask*>(deletedNode.data()) &&
        deletedNode == m_d->isolationRootNode) {

        Q_EMIT sigInternalStopIsolatedModeRequested();
    }

    KisLayerUtils::recursiveApplyNodes(KisSharedPtr<KisNode>(parent), [this](KisNodeSP node){
       QMap<QString, KisKeyframeChannel*> chans = node->keyframeChannels();
       Q_FOREACH(KisKeyframeChannel* chan, chans.values()) {
           this->keyframeChannelAboutToBeRemoved(node.data(), chan);
       }
    });

    KisNodeGraphListener::aboutToRemoveANode(parent, index);

    SANITY_CHECK_LOCKED("aboutToRemoveANode");
    m_d->signalRouter.emitAboutToRemoveANode(parent, index);
}

void KisImage::nodeChanged(KisNode* node)
{
    KisNodeGraphListener::nodeChanged(node);
    m_d->signalRouter.emitNodeChanged(node);
}

void KisImage::invalidateAllFrames()
{
    invalidateFrames(KisTimeSpan::infinite(0), QRect());
}

void KisImage::setOverlaySelectionMask(KisSelectionMaskSP mask)
{
    if (m_d->targetOverlaySelectionMask == mask) return;

    m_d->targetOverlaySelectionMask = mask;

    struct UpdateOverlaySelectionStroke : public KisSimpleStrokeStrategy {
        UpdateOverlaySelectionStroke(KisImageSP image)
            : KisSimpleStrokeStrategy(QLatin1String("update-overlay-selection-mask"), kundo2_noi18n("update-overlay-selection-mask")),
              m_image(image)
        {
            this->enableJob(JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
            setClearsRedoOnStart(false);
        }

        void initStrokeCallback() override {
            KisSelectionMaskSP oldMask = m_image->m_d->overlaySelectionMask;
            KisSelectionMaskSP newMask = m_image->m_d->targetOverlaySelectionMask;
            if (oldMask == newMask) return;

            KIS_SAFE_ASSERT_RECOVER_RETURN(!newMask || static_cast<KisImage*>(newMask->graphListener()) == m_image);

            m_image->m_d->overlaySelectionMask = newMask;

            if (oldMask || newMask) {
                m_image->m_d->rootLayer->notifyChildMaskChanged();
            }

            if (oldMask) {
                const QRect oldMaskRect = oldMask->graphListener() ? oldMask->extent() : m_image->bounds();
                m_image->m_d->rootLayer->setDirtyDontResetAnimationCache(oldMaskRect);
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

bool KisImage::canReselectGlobalSelection()
{
    return m_d->deselectedGlobalSelectionMask;
}

KisImageGlobalSelectionManagementInterface* KisImage::globalSelectionManagementInterface() const
{
    return &m_d->globalSelectionInterface;
}

QString KisImage::nextLayerName(const QString &_baseName) const
{
    QString baseName = _baseName;

    int numLayers = 0;
    int maxLayerIndex = 0;
    QRegularExpression numberedLayerRegexp(".* (\\d+)$");
    KisLayerUtils::recursiveApplyNodes(root(),
        [&numLayers, &maxLayerIndex, &numberedLayerRegexp] (KisNodeSP node) {
            if (node->inherits("KisLayer")) {
                QRegularExpressionMatch match = numberedLayerRegexp.match(node->name());

                if (match.hasMatch()) {
                    maxLayerIndex = qMax(maxLayerIndex, match.captured(1).toInt());
                }
                numLayers++;
            }
        });

    // special case if there is only root node
    if (numLayers == 1) {
        return i18nc("Name for the bottom-most layer in the layerstack", "Background");
    }

    if (baseName.isEmpty()) {
        baseName = i18n("Paint Layer");
    }

    return QString("%1 %2").arg(baseName).arg(maxLayerIndex + 1);
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
        KisBusyWaitBroker::instance()->notifyWaitOnImageStarted(this);
        m_d->scheduler.barrierLock();
        KisBusyWaitBroker::instance()->notifyWaitOnImageEnded(this);
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

void KisImage::immediateLockForReadOnly()
{
    if (!locked()) {
        requestStrokeEnd();
        KisBusyWaitBroker::instance()->notifyWaitOnImageStarted(this);
        m_d->scheduler.immediateLockForReadOnly();
        KisBusyWaitBroker::instance()->notifyWaitOnImageEnded(this);
    }
    m_d->lockCount++;
    m_d->lockedForReadOnly = true;
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

void KisImage::purgeUnusedData(bool isCancellable)
{
    /**
     * WARNING: don't use this function unless you know what you are doing!
     *
     * It breaks undo on layers! Therefore, after calling it, KisImage is not
     * undo-capable anymore!
     */

    struct PurgeUnusedDataStroke : public KisRunnableBasedStrokeStrategy {
        PurgeUnusedDataStroke(KisImageSP image, bool isCancellable)
            : KisRunnableBasedStrokeStrategy(QLatin1String("purge-unused-data"),
                                             kundo2_noi18n("purge-unused-data")),
              m_image(image)
        {
            this->enableJob(JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
            this->enableJob(JOB_DOSTROKE, true);
            setClearsRedoOnStart(false);
            setRequestsOtherStrokesToEnd(!isCancellable);
            setCanForgetAboutMe(isCancellable);
        }

        void initStrokeCallback() override
        {
            KisPaintDeviceList deviceList;
            QVector<KisStrokeJobData*> jobsData;

            KisLayerUtils::recursiveApplyNodes(m_image->root(),
                [&deviceList](KisNodeSP node) {
                   deviceList << node->getLodCapableDevices();
                 });

            /// make sure we deduplicate the list to avoid
            /// concurrent write access to the devices
            KritaUtils::makeContainerUnique(deviceList);

            Q_FOREACH (KisPaintDeviceSP device, deviceList) {
                if (!device) continue;

                KritaUtils::addJobConcurrent(jobsData,
                    [device] () {
                        const_cast<KisPaintDevice*>(device.data())->purgeDefaultPixels();
                    });
            }

            addMutatedJobs(jobsData);
        }

    private:
        KisImageSP m_image;
    };

    KisStrokeId id = startStroke(new PurgeUnusedDataStroke(this, isCancellable));
    endStroke(id);
}

void KisImage::cropNode(KisNodeSP node, const QRect& newRect, const bool activeFrameOnly)
{
    const bool isLayer = qobject_cast<KisLayer*>(node.data());
    KUndo2MagicString actionName = isLayer ?
        kundo2_i18n("Crop Layer") :
        kundo2_i18n("Crop Mask");

    KisImageSignalVector emitSignals;

    KisCropSavedExtraData *extraData =
        new KisCropSavedExtraData(KisCropSavedExtraData::CROP_LAYER,
                                  newRect, node);

    KisProcessingApplicator applicator(this, node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName, extraData);

    KisProcessingVisitorSP visitor =
        new KisCropProcessingVisitor(newRect, true, false);

    if (node->isAnimated() && activeFrameOnly) {
        // Crop active frame..
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    } else {
        // Crop all frames..
        applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    }
    applicator.end();
}

void KisImage::scaleImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy)
{
    bool resolutionChanged = !qFuzzyCompare(xRes(), xres) || !qFuzzyCompare(yRes(), yres);
    bool sizeChanged = size != this->size();

    if (!resolutionChanged && !sizeChanged) return;

    KisImageSignalVector emitSignals;
    if (resolutionChanged) emitSignals << ResolutionChangedSignal;
    if (sizeChanged) emitSignals << ComplexSizeChangedSignal(bounds(), size);

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
    scaleNodes(KisNodeList{node}, center, scaleX, scaleY, filterStrategy, selection);
}
void KisImage::scaleNodes(KisNodeList nodes, const QPointF &center, qreal scaleX, qreal scaleY, KisFilterStrategy *filterStrategy, KisSelectionSP selection)
{
    KUndo2MagicString actionName(kundo2_i18np("Scale Layer", "Scale %1 Layers", nodes.size()));
    KisImageSignalVector emitSignals;

    QPointF offset;
    {
        KisTransformWorker worker(0,
                                  scaleX, scaleY,
                                  0, 0,
                                  0.0,
                                  0, 0, 0, 0);
        QTransform transform = worker.transform();

        offset = center - transform.map(center);
    }

    KisProcessingApplicator applicator(this, nodes,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    KisTransformProcessingVisitor *visitor =
        new KisTransformProcessingVisitor(scaleX, scaleY,
                                          0, 0,
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
    rotateImpl(actionName, KisNodeList{rootNode}, radians, resizeImage, selection);
}
void KisImage::rotateImpl(const KUndo2MagicString &actionName,
                          KisNodeList nodes,
                          double radians,
                          bool resizeImage,
                          KisSelectionSP selection)
{
    // we can either transform (and resize) the whole image or
    // transform a selection, we cannot do both at the same time
    KIS_SAFE_ASSERT_RECOVER(!(bool(selection) && resizeImage)) {
        selection = 0;
    }

    QRect baseBounds;
    if (resizeImage) {
        baseBounds = bounds();
    }
    else if (selection) {
        baseBounds = selection->selectedExactRect();
    }
    else {
        Q_FOREACH(KisNodeSP node, nodes) {
            baseBounds = baseBounds.united(node->exactBounds());
        }
    }

    QPointF offset;
    QSize newSize;

    {
        KisTransformWorker worker(0,
                                  1.0, 1.0,
                                  0, 0,
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

    // These flags determine whether updates are transferred to the UI during processing
    KisProcessingApplicator::ProcessingFlags signalFlags =
        sizeChanged ?
        KisProcessingApplicator::NO_UI_UPDATES :
        KisProcessingApplicator::NONE;


    KisProcessingApplicator applicator(this, nodes,
                                       KisProcessingApplicator::RECURSIVE | signalFlags,
                                       emitSignals, actionName);

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Bicubic");

    KisTransformProcessingVisitor *visitor =
            new KisTransformProcessingVisitor(1.0, 1.0, 0.0, 0.0,
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
    rotateNodes(KisNodeList{node}, radians, selection);
}
void KisImage::rotateNodes(KisNodeList nodes, double radians, KisSelectionSP selection)
{
    if (nodes.size() == 1 && nodes[0]->inherits("KisMask")) {
        rotateImpl(kundo2_i18n("Rotate Mask"), nodes, radians, false, selection);
    }
    else {
        rotateImpl(kundo2_i18np("Rotate Layer", "Rotate %1 Layers", nodes.size()), nodes, radians, false, selection);
    }
}

void KisImage::shearImpl(const KUndo2MagicString &actionName,
                         KisNodeSP rootNode,
                         bool resizeImage,
                         double angleX, double angleY,
                         KisSelectionSP selection)
{
    shearImpl(actionName, KisNodeList{rootNode}, resizeImage, angleX, angleY, selection);
}
void KisImage::shearImpl(const KUndo2MagicString &actionName,
                         KisNodeList nodes,
                         bool resizeImage,
                         double angleX, double angleY,
                         KisSelectionSP selection)
{
    QRect baseBounds;
    if (resizeImage) {
        baseBounds = bounds();
    }
    else if (selection) {
        baseBounds = selection->selectedExactRect();
    }
    else {
        Q_FOREACH(KisNodeSP node, nodes) {
            baseBounds = baseBounds.united(node->exactBounds());
        }
    }
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
                                  tanX, tanY,
                                  0,
                                  0, 0, 0, 0);

        QRect newRect = worker.transform().mapRect(baseBounds);
        newSize = newRect.size();
        if (resizeImage) offset = -newRect.topLeft();
        else offset = origin - worker.transform().map(origin);
    }

    if (newSize == baseBounds.size()) return;

    KisImageSignalVector emitSignals;
    if (resizeImage) emitSignals << ComplexSizeChangedSignal(baseBounds, newSize);

    KisProcessingApplicator::ProcessingFlags signalFlags =
        KisProcessingApplicator::RECURSIVE;
    if (resizeImage) signalFlags |= KisProcessingApplicator::NO_UI_UPDATES;

    KisProcessingApplicator applicator(this, nodes,
                                       signalFlags,
                                       emitSignals, actionName);

    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value("Bilinear");

    KisTransformProcessingVisitor *visitor =
            new KisTransformProcessingVisitor(1.0, 1.0,
                                              tanX, tanY,
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
    shearNodes(KisNodeList{node}, angleX, angleY, selection);
}
void KisImage::shearNodes(KisNodeList nodes, double angleX, double angleY, KisSelectionSP selection)
{
    if (nodes.size() == 1 && nodes[0]->inherits("KisMask")) {
        shearImpl(kundo2_i18n("Shear Mask"), nodes, false,
                  angleX, angleY, selection);
    }
    else {
        shearImpl(kundo2_i18np("Shear Layer", "Shear %1 Layers", nodes.size()), nodes, false,
                  angleX, angleY, selection);
    }
}

void KisImage::shear(double angleX, double angleY)
{
    shearImpl(kundo2_i18n("Shear Image"), m_d->rootLayer, true,
              angleX, angleY, 0);
}

void KisImage::convertLayerColorSpace(KisNodeSP node,
                                      const KoColorSpace *dstColorSpace,
                                      KoColorConversionTransformation::Intent renderingIntent,
                                      KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    if (!node->projectionLeaf()->isLayer()) return;
    // must not be an image root, use convertImageColorSpace() for that:
    KIS_SAFE_ASSERT_RECOVER_RETURN(!node->image() || (node.data() != node->image()->rootLayer().data()));

    const KoColorSpace *srcColorSpace = node->colorSpace();

    if (!dstColorSpace || *srcColorSpace == *dstColorSpace) return;

    KUndo2MagicString actionName =
        kundo2_i18n("Convert Layer Color Space");

    KisImageSignalVector emitSignals;

    KisProcessingApplicator applicator(this, node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    applicator.applyVisitor(
        new KisConvertColorSpaceProcessingVisitor(
            srcColorSpace, dstColorSpace,
            renderingIntent, conversionFlags),
        KisStrokeJobData::CONCURRENT);

    applicator.end();
}

struct KisImage::KisImagePrivate::SetImageProjectionColorSpace : public KisCommandUtils::FlipFlopCommand
{
    SetImageProjectionColorSpace(const KoColorSpace *cs, KisImageWSP image,
                                 State initialState, KUndo2Command *parent = 0)
        : KisCommandUtils::FlipFlopCommand(initialState, parent),
          m_cs(cs),
          m_image(image)
    {
    }

    void partA() override {
        KisImageSP image = m_image;

        if (image) {
            image->setProjectionColorSpace(m_cs);
        }
    }

private:
    const KoColorSpace *m_cs;
    KisImageWSP m_image;
};

void KisImage::KisImagePrivate::convertImageColorSpaceImpl(const KoColorSpace *dstColorSpace,
                                                           bool convertLayers,
                                                           KoColorConversionTransformation::Intent renderingIntent,
                                                           KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    const KoColorSpace *srcColorSpace = this->colorSpace;

    if (!dstColorSpace || *srcColorSpace == *dstColorSpace) return;

    const KUndo2MagicString actionName =
        convertLayers ?
        kundo2_i18n("Convert Image Color Space") :
        kundo2_i18n("Convert Projection Color Space");

    KisImageSignalVector emitSignals;
    emitSignals << ColorSpaceChangedSignal;

    KisProcessingApplicator::ProcessingFlags flags = KisProcessingApplicator::NO_UI_UPDATES;
    if (convertLayers) {
        flags |= KisProcessingApplicator::RECURSIVE;
    }

    KisProcessingApplicator applicator(q, this->rootLayer,
                                       flags,
                                       emitSignals, actionName);

    applicator.applyCommand(
        new KisImagePrivate::SetImageProjectionColorSpace(dstColorSpace,
                                                          KisImageWSP(q),
                                                          KisCommandUtils::FlipFlopCommand::INITIALIZING),
        KisStrokeJobData::BARRIER);

    applicator.applyVisitor(
                new KisConvertColorSpaceProcessingVisitor(
                    srcColorSpace, dstColorSpace,
                    renderingIntent, conversionFlags),
                KisStrokeJobData::CONCURRENT);

    applicator.applyCommand(
        new KisImagePrivate::SetImageProjectionColorSpace(srcColorSpace,
                                                          KisImageWSP(q),
                                                          KisCommandUtils::FlipFlopCommand::FINALIZING),
        KisStrokeJobData::BARRIER);


    applicator.end();
}

void KisImage::convertImageColorSpace(const KoColorSpace *dstColorSpace,
                                      KoColorConversionTransformation::Intent renderingIntent,
                                      KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    m_d->convertImageColorSpaceImpl(dstColorSpace, true, renderingIntent, conversionFlags);
}

void KisImage::convertImageProjectionColorSpace(const KoColorSpace *dstColorSpace)
{
    m_d->convertImageColorSpaceImpl(dstColorSpace, false,
                                    KoColorConversionTransformation::internalRenderingIntent(),
                                    KoColorConversionTransformation::internalConversionFlags());
}

void KisImage::unifyLayersColorSpace()
{
    const KUndo2MagicString actionName = kundo2_i18n("Unify Layers Color Space");

    KisImageSignalVector emitSignals;

    KisProcessingApplicator::ProcessingFlags flags =
        KisProcessingApplicator::NO_UI_UPDATES | KisProcessingApplicator::RECURSIVE;

    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       flags,
                                       emitSignals, actionName);

    // src and dst color spaces coincide, since we should just unify
    // all our layers
    applicator.applyVisitor(
                new KisConvertColorSpaceProcessingVisitor(
                    m_d->colorSpace, m_d->colorSpace,
                    KoColorConversionTransformation::internalRenderingIntent(),
                    KoColorConversionTransformation::internalConversionFlags()),
                KisStrokeJobData::CONCURRENT);

    applicator.end();
}

bool KisImage::assignLayerProfile(KisNodeSP node, const KoColorProfile *profile)
{
    const KoColorSpace *srcColorSpace = node->colorSpace();

    if (!node->projectionLeaf()->isLayer()) return false;
    if (!profile || *srcColorSpace->profile() == *profile) return false;

    KUndo2MagicString actionName = kundo2_i18n("Assign Profile to Layer");

    KisImageSignalVector emitSignals;

    const KoColorSpace *dstColorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (!dstColorSpace) return false;

    KisProcessingApplicator applicator(this, node,
                                       KisProcessingApplicator::RECURSIVE |
                                       KisProcessingApplicator::NO_UI_UPDATES,
                                       emitSignals, actionName);

    applicator.applyVisitor(
        new KisAssignProfileProcessingVisitor(
            srcColorSpace, dstColorSpace),
        KisStrokeJobData::CONCURRENT);

    applicator.end();

    return true;
}


bool KisImage::assignImageProfile(const KoColorProfile *profile, bool blockAllUpdates)
{
    if (!profile) return false;

    const KoColorSpace *srcColorSpace = m_d->colorSpace;
    bool imageProfileIsSame = *srcColorSpace->profile() == *profile;

    imageProfileIsSame &=
        !KisLayerUtils::recursiveFindNode(m_d->rootLayer,
            [profile] (KisNodeSP node) {
                return *node->colorSpace()->profile() != *profile;
            });

    if (imageProfileIsSame) {
        dbgImage << "Trying to set the same image profile again" << ppVar(srcColorSpace->profile()->name()) << ppVar(profile->name());
        return true;
    }

    KUndo2MagicString actionName = kundo2_i18n("Assign Profile");

    KisImageSignalVector emitSignals;
    emitSignals << ProfileChangedSignal;

    const KoColorSpace *dstColorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (!dstColorSpace) return false;

    KisProcessingApplicator applicator(this, m_d->rootLayer,
                                       KisProcessingApplicator::RECURSIVE |
                                       (!blockAllUpdates ?
                                            KisProcessingApplicator::NO_UI_UPDATES :
                                            KisProcessingApplicator::NO_IMAGE_UPDATES),
                                       emitSignals, actionName);

    applicator.applyCommand(
        new KisImagePrivate::SetImageProjectionColorSpace(dstColorSpace,
                                                          KisImageWSP(this),
                                                          KisCommandUtils::FlipFlopCommand::INITIALIZING),
        KisStrokeJobData::BARRIER);

    applicator.applyVisitor(
        new KisAssignProfileProcessingVisitor(
            srcColorSpace, dstColorSpace),
        KisStrokeJobData::CONCURRENT);

    applicator.applyCommand(
        new KisImagePrivate::SetImageProjectionColorSpace(srcColorSpace,
                                                          KisImageWSP(this),
                                                          KisCommandUtils::FlipFlopCommand::FINALIZING),
        KisStrokeJobData::BARRIER);


    applicator.end();

    return true;
}

void KisImage::setProjectionColorSpace(const KoColorSpace * colorSpace)
{
    m_d->colorSpace = colorSpace;
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
    if (xres > 0) {
        m_d->xres = xres;
    } else {
        qWarning() << "WARNING: Ignoring attempt to set image x resolution <= 0 (" << xres << ")!";
    }

    if (yres > 0) {
        m_d->yres = yres;
    } else {
        qWarning() << "WARNING: Ignoring attempt to set image y resolution <= 0 (" << yres << ")!";
    }
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
    if (m_d->isolationRootNode) {
        return m_d->isolationRootNode->projection();
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

qint32 KisImage::nChildLayers() const
{
    const QStringList list = {"KisLayer"};

    KoProperties koProperties;
    KisCountVisitor visitor(list, koProperties);
    const QList<KisNodeSP> childNodes = m_d->rootLayer->childNodes(list, koProperties);
    for (KisNodeSP childNode: childNodes) {
        childNode->accept(visitor);
    }
    return visitor.count();
}

void KisImage::flatten(KisNodeSP activeNode)
{
    KisLayerUtils::flattenImage(this, activeNode);
}

void KisImage::mergeMultipleLayers(QList<KisNodeSP> mergedNodes, KisNodeSP putAfter)
{
    KisLayerUtils::mergeMultipleNodes(this, mergedNodes, putAfter);
}

void KisImage::mergeDown(KisLayerSP layer, const KisMetaData::MergeStrategy* strategy)
{
    KisLayerUtils::mergeDown(this, layer, strategy);
}

void KisImage::flattenLayer(KisLayerSP layer)
{
    KisLayerUtils::flattenLayer(this, layer);
}

void KisImage::setModifiedWithoutUndo()
{
    m_d->signalRouter.emitNotification(ModifiedWithoutUndoSignal);
    Q_EMIT sigImageModified();
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


    if (scaleX < 1.0/256 || scaleY < 1.0/256) {
        // quick checking if we're not trying to scale too much
        // convertToQImage uses KisFixedPoint values, which means that the scale cannot be smaller than 1/2^8
        // BUG:432182
        // FIXME: would be best to extend KisFixedPoint instead
        return convertToQImage(size(), profile).scaled(scaledImageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    KoDummyUpdaterHolder updaterHolder;
    QPointer<KoUpdater> updater = updaterHolder.updater();

    KisTransformWorker worker(dev, scaleX, scaleY, 0.0, 0.0, 0.0, 0.0, 0.0, updater, KisFilterStrategyRegistry::instance()->value("Bicubic"));
    worker.run();

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
    disconnect(m_d->undoStore.data(), SIGNAL(historyStateChanged()), &m_d->signalRouter, SLOT(emitImageModifiedNotification()));

    m_d->legacyUndoAdapter.setUndoStore(undoStore);
    m_d->postExecutionUndoAdapter.setUndoStore(undoStore);
    m_d->undoStore.reset(undoStore);

    connect(m_d->undoStore.data(), SIGNAL(historyStateChanged()), &m_d->signalRouter, SLOT(emitImageModifiedNotification()));

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
        return KoColor::createTransparent(m_d->colorSpace);
    }

    return m_d->rootLayer->defaultProjectionColor();
}

void KisImage::setRootLayer(KisGroupLayerSP rootLayer)
{
    Q_EMIT sigInternalStopIsolatedModeRequested();

    KoColor defaultProjectionColor = KoColor::createTransparent(m_d->colorSpace);

    if (m_d->rootLayer) {
        m_d->rootLayer->setGraphListener(0);
        m_d->rootLayer->setImage(0);
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
            setModifiedWithoutUndo();
            return;
        }
        ++it;
    }
    m_d->annotations.push_back(annotation);
    setModifiedWithoutUndo();
}

KisAnnotationSP KisImage::annotation(const QString& type)
{
    vKisAnnotationSP_it it = m_d->annotations.begin();
    while (it != m_d->annotations.end()) {
        if ((*it) && (*it)->type() == type) {
            return *it;
        }
        else if (!*it) {
            qWarning() << "Skipping deleted annotation";
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
            setModifiedWithoutUndo();
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
    Q_EMIT sigAboutToBeDeleted();
}

KisImageSignalRouter* KisImage::signalRouter()
{
    return &m_d->signalRouter;
}

void KisImage::waitForDone()
{
    requestStrokeEnd();
    KisBusyWaitBroker::instance()->notifyWaitOnImageStarted(this);
    m_d->scheduler.waitForDone();
    KisBusyWaitBroker::instance()->notifyWaitOnImageEnded(this);
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

bool KisImage::startIsolatedMode(KisNodeSP node, bool isolateLayer, bool isolateGroup)
{
    m_d->isolateLayer = isolateLayer;
    m_d->isolateGroup = isolateGroup;
    if ((isolateLayer || isolateGroup) == false) return false;

    /**
     * Isolation of transform masks is not possible, so we should
     * not allow that
     */
    if (!node->projection()) return false;

    struct StartIsolatedModeStroke : public KisRunnableBasedStrokeStrategy {
        StartIsolatedModeStroke(KisNodeSP node, KisImageSP image, bool isolateLayer, bool isolateGroup)
            : KisRunnableBasedStrokeStrategy(QLatin1String("start-isolated-mode"),
                                             kundo2_noi18n("start-isolated-mode")),
              m_newRoot(node),
              m_image(image),
              m_isolateLayer(isolateLayer),
              m_isolateGroup(isolateGroup)
        {
            this->enableJob(JOB_INIT, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
            this->enableJob(JOB_DOSTROKE, true);
            this->enableJob(JOB_FINISH, true, KisStrokeJobData::BARRIER);
            setClearsRedoOnStart(false);
        }

        void initStrokeCallback() override {
            if (m_isolateLayer == false && m_isolateGroup == true) {
                // Isolate parent node unless node is the root note.
                m_newRoot = m_newRoot->parent() ? m_newRoot->parent() : m_newRoot;
            }
            // pass-though node don't have any projection prepared, so we should
            // explicitly regenerate it before activating isolated mode.
            m_newRoot->projectionLeaf()->explicitlyRegeneratePassThroughProjection();
            m_prevRoot = m_image->m_d->isolationRootNode;

            const bool beforeVisibility = m_newRoot->projectionLeaf()->visible();
            const bool prevRootBeforeVisibility = m_prevRoot ? m_prevRoot->projectionLeaf()->visible() : false;

            m_image->m_d->isolationRootNode = m_newRoot;
            Q_EMIT m_image->sigIsolatedModeChanged();

            const bool afterVisibility = m_newRoot->projectionLeaf()->visible();
            const bool prevRootAfterVisibility = m_prevRoot ? m_prevRoot->projectionLeaf()->visible() : false;

            m_newRootNeedsFullRefresh = beforeVisibility != afterVisibility;
            m_prevRootNeedsFullRefresh = prevRootBeforeVisibility != prevRootAfterVisibility;
        }

        void finishStrokeCallback() override {
            // the GUI uses our thread to do the color space conversion so we
            // need to Q_EMIT this signal in multiple threads

            if (m_prevRoot && m_prevRootNeedsFullRefresh) {
                m_image->refreshGraphAsync(m_prevRoot);
            }

            if (m_newRootNeedsFullRefresh) {
                m_image->refreshGraphAsync(m_newRoot);
            }

            if (!m_prevRootNeedsFullRefresh && !m_newRootNeedsFullRefresh) {
                QVector<KisRunnableStrokeJobData*> jobs;
                m_image->m_d->notifyProjectionUpdatedInPatches(m_image->bounds(), jobs);
                this->runnableJobsInterface()->addRunnableJobs(jobs);
            }

            m_image->invalidateAllFrames();
        }

    private:
        KisNodeSP m_newRoot;
        KisNodeSP m_prevRoot;
        KisImageSP m_image;
        bool m_newRootNeedsFullRefresh = false;
        bool m_prevRootNeedsFullRefresh = false;

        bool m_isolateLayer;
        bool m_isolateGroup;
    };

    KisStrokeId id = startStroke(new StartIsolatedModeStroke(node, this, isolateLayer, isolateGroup));
    endStroke(id);

    return true;
}

void KisImage::stopIsolatedMode()
{
    if (!m_d->isolationRootNode)  return;

    struct StopIsolatedModeStroke : public KisRunnableBasedStrokeStrategy {
        StopIsolatedModeStroke(KisImageSP image)
            : KisRunnableBasedStrokeStrategy(QLatin1String("stop-isolated-mode"), kundo2_noi18n("stop-isolated-mode")),
              m_image(image),
              m_oldRootNode(nullptr),
              m_oldNodeNeedsRefresh(false)
        {
            this->enableJob(JOB_INIT);
            this->enableJob(JOB_DOSTROKE, true);
            this->enableJob(JOB_FINISH, true, KisStrokeJobData::BARRIER);
            setClearsRedoOnStart(false);
        }

        void initStrokeCallback() override {
            if (!m_image->m_d->isolationRootNode)  return;

            m_oldRootNode = m_image->m_d->isolationRootNode;

            const bool beforeVisibility = m_oldRootNode->projectionLeaf()->visible();
            m_image->m_d->isolationRootNode = 0;
            m_image->m_d->isolateLayer = false;
            m_image->m_d->isolateGroup = false;
            Q_EMIT m_image->sigIsolatedModeChanged();
            const bool afterVisibility = m_oldRootNode->projectionLeaf()->visible();

            m_oldNodeNeedsRefresh = (beforeVisibility != afterVisibility);
        }

        void finishStrokeCallback() override {

            m_image->invalidateAllFrames();

            if (m_oldNodeNeedsRefresh){
                m_oldRootNode->setDirty(m_image->bounds());
            } else {
                // TODO: Substitute notifyProjectionUpdated() with this code
                // when update optimization is implemented
                //
                // QRect updateRect = bounds() | oldRootNode->extent();
                //oldRootNode->setDirty(updateRect);

                QVector<KisRunnableStrokeJobData*> jobs;
                m_image->m_d->notifyProjectionUpdatedInPatches(m_image->bounds(), jobs);
                this->runnableJobsInterface()->addRunnableJobs(jobs);
            }
        }

    private:
        KisImageSP m_image;
        KisNodeSP m_oldRootNode;
        bool m_oldNodeNeedsRefresh;
    };

    KisStrokeId id = startStroke(new StopIsolatedModeStroke(this));
    endStroke(id);
}

KisNodeSP KisImage::isolationRootNode() const {
    return m_d->isolationRootNode;
}

bool KisImage::isIsolatingLayer() const
{
    return m_d->isolateLayer;
}

bool KisImage::isIsolatingGroup() const
{
     return m_d->isolateGroup;
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
    Q_EMIT sigUndoDuringStrokeRequested();
}

void KisImage::requestRedoDuringStroke()
{
    Q_EMIT sigRedoDuringStrokeRequested();
}

void KisImage::requestStrokeCancellation()
{
    if (!m_d->tryCancelCurrentStrokeAsync()) {
        Q_EMIT sigStrokeCancellationRequested();
    }
}

UndoResult KisImage::tryUndoUnfinishedLod0Stroke()
{
    return m_d->scheduler.tryUndoLastStrokeAsync();
}

void KisImage::requestStrokeEnd()
{
    Q_EMIT sigStrokeEndRequested();
    Q_EMIT sigStrokeEndRequestedActiveNodeFiltered();
}

void KisImage::requestStrokeEndActiveNode()
{
    Q_EMIT sigStrokeEndRequested();
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

void KisImage::refreshGraphAsync(KisNodeSP root, const QVector<QRect> &rects, const QRect &cropRect, KisProjectionUpdateFlags flags)
{
    if (!root) root = m_d->rootLayer;

    QVector<QRect> requestedRects = rects;

    KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(root.data());
    if (group && group->passThroughMode()) {
        /**
         * We cannot regenerate a pass-through group, since it is not present in
         * the layers stack. Instead we should delegate this regeneration pass
         * to the parent group.
         *
         * We should also take into account the change rect of all the layers that lay
         * above the "dirty" pass-through group (i.e. convert the change rect of the
         * child into a change rect of the parent). That is required for a case,
         * when there are adjustment layers laying above the pass-through group.
         */
        QVector<QRect> changeRects = requestedRects;
        KisProjectionLeafSP leaf = root->projectionLeaf()->nextSibling();
        while (leaf) {
            if (leaf->shouldBeRendered()) {
                for (auto it = changeRects.begin(); it != changeRects.end(); ++it) {
                    *it = leaf->projectionPlane()->changeRect(*it, leaf->node() == root ? KisNode::N_FILTHY : KisNode::N_ABOVE_FILTHY);
                }
            }

            leaf = leaf->nextSibling();
        }

        std::swap(requestedRects, changeRects);
        root = group->parent();

        KIS_SAFE_ASSERT_RECOVER_RETURN(root);
    }

    /**
     * We iterate through the filters in a reversed way. It makes the most nested filters
     * to execute first.
     */
    for (auto it = m_d->projectionUpdatesFilters.rbegin();
         it != m_d->projectionUpdatesFilters.rend();
         ++it) {

        KIS_SAFE_ASSERT_RECOVER(*it) { continue; }

        if ((*it)->filterRefreshGraph(this, root.data(), requestedRects, cropRect, flags)) {
            return;
        }
    }

    if (!flags.testFlag(KisProjectionUpdateFlag::DontInvalidateFrames)) {
        m_d->animationInterface->notifyNodeChanged(root.data(), requestedRects, true);
    }

    m_d->scheduler.fullRefreshAsync(root, requestedRects, cropRect, flags);
}

void KisImage::addSpontaneousJob(KisSpontaneousJob *spontaneousJob)
{
    m_d->scheduler.addSpontaneousJob(spontaneousJob);
}

bool KisImage::hasUpdatesRunning() const
{
    return m_d->scheduler.hasUpdatesRunning();
}

KisProjectionUpdatesFilterCookie KisImage::addProjectionUpdatesFilter(KisProjectionUpdatesFilterSP filter)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(filter, KisProjectionUpdatesFilterCookie());

    m_d->projectionUpdatesFilters.append(filter);

    return KisProjectionUpdatesFilterCookie(filter.data());
}

KisProjectionUpdatesFilterSP KisImage::removeProjectionUpdatesFilter(KisProjectionUpdatesFilterCookie cookie)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(cookie);
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->projectionUpdatesFilters.last() == cookie);

    auto it = std::find(m_d->projectionUpdatesFilters.begin(), m_d->projectionUpdatesFilters.end(), cookie);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it != m_d->projectionUpdatesFilters.end(), KisProjectionUpdatesFilterSP());

    KisProjectionUpdatesFilterSP filter = *it;

    m_d->projectionUpdatesFilters.erase(it);

    return filter;
}

KisProjectionUpdatesFilterCookie KisImage::currentProjectionUpdatesFilter() const
{
    return !m_d->projectionUpdatesFilters.isEmpty() ?
                m_d->projectionUpdatesFilters.last().data() :
                KisProjectionUpdatesFilterCookie();
}

void KisImage::disableDirtyRequests()
{
    m_d->disabledUpdatesCookies.push(
        addProjectionUpdatesFilter(toQShared(new KisDropAllProjectionUpdatesFilter())));
}

void KisImage::enableDirtyRequests()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->disabledUpdatesCookies.isEmpty());
    removeProjectionUpdatesFilter(m_d->disabledUpdatesCookies.pop());
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

        Q_EMIT sigImageUpdated(dirtyRect);
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
     * The selection is calculated asynchronously, so it is not
     * handled by disableUIUpdates() and other special signals of
     * KisImageSignalRouter
     */
    m_d->legacyUndoAdapter.emitSelectionChanged();

    /**
     * Editing of selection masks doesn't necessary produce a
     * setDirty() call, so in the end of the stroke we need to request
     * direct update of the UI's cache.
     */
    if (m_d->isolationRootNode &&
        dynamic_cast<KisSelectionMask*>(m_d->isolationRootNode.data())) {

        notifyProjectionUpdated(bounds());
    }
}

void KisImage::KisImagePrivate::
    requestProjectionUpdateImpl(KisNode *node,
                                const QVector<QRect> &rects,
                                const QRect &cropRect,
                                KisProjectionUpdateFlags flags)
{
    if (rects.isEmpty()) return;

    scheduler.updateProjection(node, rects, cropRect, flags);
}

void KisImage::requestProjectionUpdate(KisNode *node, const QVector<QRect> &rects, KisProjectionUpdateFlags flags)
{
    /**
     * We iterate through the filters in a reversed way. It makes the most nested filters
     * to execute first.
     */
    for (auto it = m_d->projectionUpdatesFilters.rbegin();
         it != m_d->projectionUpdatesFilters.rend();
         ++it) {

        KIS_SAFE_ASSERT_RECOVER(*it) { continue; }

        if ((*it)->filter(this, node, rects, flags)) {
            return;
        }
    }

    if (!flags.testFlag(KisProjectionUpdateFlag::DontInvalidateFrames)) {
        m_d->animationInterface->notifyNodeChanged(node, rects, false);
    }

    /**
     * Here we use 'permitted' instead of 'active' intentively,
     * because the updates may come after the actual stroke has been
     * finished. And having some more updates for the stroke not
     * supporting the wrap-around mode will not make much harm.
     */
    if (m_d->wrapAroundModePermitted && !flags.testFlag(KisProjectionUpdateFlag::NoFilthy)) {
        /// TODO: Remove the no-filthy condition. I have no idea, why it is present.
        ///       Or replace it with an assert. (DK)
        QVector<QRect> allSplitRects;

        const QRect boundRect = effectiveLodBounds();
        Q_FOREACH (const QRect &rc, rects) {
            KisWrappedRect splitRect(rc, boundRect, m_d->wrapAroundModeAxis);
            allSplitRects.append(splitRect);
        }

        m_d->requestProjectionUpdateImpl(node, allSplitRects, boundRect, flags);

    } else {
        m_d->requestProjectionUpdateImpl(node, rects, bounds(), flags);
    }

    KisNodeGraphListener::requestProjectionUpdate(node, rects, flags);
}

void KisImage::invalidateFrames(const KisTimeSpan &range, const QRect &rect)
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

void KisImage::keyframeChannelHasBeenAdded(KisNode *node, KisKeyframeChannel *channel)
{
    Q_UNUSED(node);

    channel->connect(channel, SIGNAL(sigAddedKeyframe(const KisKeyframeChannel*, int)), m_d->animationInterface, SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*, int)), Qt::UniqueConnection);
    channel->connect(channel, SIGNAL(sigKeyframeHasBeenRemoved(const KisKeyframeChannel*,int)), m_d->animationInterface, SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*, int)), Qt::UniqueConnection);
}

void KisImage::keyframeChannelAboutToBeRemoved(KisNode *node, KisKeyframeChannel *channel)
{
    Q_UNUSED(node);

    channel->disconnect(channel, SIGNAL(sigAddedKeyframe(const KisKeyframeChannel*, int)), m_d->animationInterface, SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*, int)));
    channel->disconnect(channel, SIGNAL(sigKeyframeHasBeenRemoved(const KisKeyframeChannel*, int)), m_d->animationInterface, SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*, int)));
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

void KisImage::moveCompositionUp(KisLayerCompositionSP composition)
{
    int index = m_d->compositions.indexOf(composition);
    if (index <= 0) {
        return;
    }
    m_d->compositions.move(index, index - 1);
}

void KisImage::moveCompositionDown(KisLayerCompositionSP composition)
{
    int index = m_d->compositions.indexOf(composition);
    if (index >= m_d->compositions.size() -1) {
        return;
    }
    m_d->compositions.move(index, index + 1);
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
                                           KisImageSignalVector(),
                                           kundo2_i18n("Crop Selections"));

        KisProcessingVisitorSP visitor =
            new KisCropSelectionsProcessingVisitor(bounds());

        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
        applicator.end();
    }
}

void KisImage::setWrapAroundModeAxis(WrapAroundAxis value)
{
    m_d->wrapAroundModeAxis = value;
}

WrapAroundAxis KisImage::wrapAroundModeAxis() const
{
    return m_d->wrapAroundModeAxis;
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

int KisImage::currentLevelOfDetail() const
{
    return m_d->scheduler.currentLevelOfDetail();
}

void KisImage::explicitRegenerateLevelOfDetail()
{
    const KisLodPreferences pref = m_d->scheduler.lodPreferences();

    if (pref.lodSupported() && pref.lodPreferred()) {
        m_d->scheduler.explicitRegenerateLevelOfDetail();
    }
}

void KisImage::setLodPreferences(const KisLodPreferences &value)
{
    m_d->scheduler.setLodPreferences(value);
}

KisLodPreferences KisImage::lodPreferences() const
{
    return m_d->scheduler.lodPreferences();
}

void KisImage::nodeCollapsedChanged(KisNode * node)
{
    Q_UNUSED(node);
    Q_EMIT sigNodeCollapsedChanged();
}

KisImageAnimationInterface* KisImage::animationInterface() const
{
    return m_d->animationInterface;
}

void KisImage::setProofingConfiguration(KisProofingConfigurationSP proofingConfig)
{
    const bool changed = bool(m_d->proofingConfig) != bool(proofingConfig) ||
        (m_d->proofingConfig && proofingConfig && *m_d->proofingConfig != *proofingConfig);

    // we still assign even when unchanged since they can be different
    // shared pointer objects
    m_d->proofingConfig = proofingConfig;

    if (changed) {
        Q_EMIT sigProofingConfigChanged();
    }
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
