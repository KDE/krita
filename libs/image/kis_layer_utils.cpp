/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_layer_utils.h"

#include <algorithm>

#include <QUuid>
#include <KoColorSpaceConstants.h>
#include <KoProperties.h>

#include "kis_painter.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "kis_group_layer.h"
#include "kis_selection.h"
#include "kis_selection_mask.h"
#include "kis_meta_data_merge_strategy.h"
#include <kundo2command.h>
#include "commands/kis_image_layer_add_command.h"
#include "commands/kis_image_layer_remove_command.h"
#include "commands/kis_image_layer_move_command.h"
#include "commands/kis_image_change_layers_command.h"
#include "commands_new/kis_activate_selection_mask_command.h"
#include "commands/kis_image_change_visibility_command.h"
#include "kis_abstract_projection_plane.h"
#include "kis_processing_applicator.h"
#include "kis_image_animation_interface.h"
#include "kis_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_projection_leaf.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_time_span.h"
#include "kis_command_utils.h"
#include "commands_new/kis_change_projection_color_command.h"
#include "kis_layer_properties_icons.h"
#include "lazybrush/kis_colorize_mask.h"
#include "commands/kis_node_property_list_command.h"
#include "commands/kis_node_compositeop_command.h"
#include <KisDelayedUpdateNodeInterface.h>
#include <KisCroppedOriginalLayerInterface.h>
#include "krita_utils.h"
#include "kis_image_signal_router.h"
#include "kis_sequential_iterator.h"
#include "kis_transparency_mask.h"
#include "kis_paint_device_frames_interface.h"


namespace KisLayerUtils {

    void fetchSelectionMasks(KisNodeList mergedNodes, QVector<KisSelectionMaskSP> &selectionMasks)
    {
        foreach (KisNodeSP node, mergedNodes) {

            Q_FOREACH(KisNodeSP child, node->childNodes(QStringList("KisSelectionMask"), KoProperties())) {

                KisSelectionMaskSP mask = qobject_cast<KisSelectionMask*>(child.data());
                if (mask) {
                    selectionMasks.append(mask);
                }
            }
        }
    }

    struct MergeDownInfoBase {
        MergeDownInfoBase(KisImageSP _image)
            : image(_image),
              storage(new SwitchFrameCommand::SharedStorage())
        {
        }
        virtual ~MergeDownInfoBase() {}

        KisImageWSP image;

        QVector<KisSelectionMaskSP> selectionMasks;

        KisNodeSP dstNode;

        SwitchFrameCommand::SharedStorageSP storage;
        QSet<int> frames;
        bool pinnedToTimeline = false;
        bool enableOnionSkins = false;

        virtual KisNodeList allSrcNodes() = 0;

        KisLayerSP dstLayer() {
            return qobject_cast<KisLayer*>(dstNode.data());
        }
    };

    struct SplitAlphaToMaskInfo {
        SplitAlphaToMaskInfo(KisImageSP _image, KisNodeSP _node, const QString& maskName)
            : image(_image)
            , node(_node)
            , storage(new SwitchFrameCommand::SharedStorage())
        {
            frames = fetchLayerFramesRecursive(_node);
            mask = new KisTransparencyMask(image, maskName);
        }

        KisImageWSP image;
        KisNodeSP node;
        SwitchFrameCommand::SharedStorageSP storage;
        QSet<int> frames;

        KisPaintDeviceSP getMaskDevice() {
            return mask->paintDevice();
        }

        KisMaskSP getMask() {
            return mask;
        }

        KisLayerSP getLayer() {
            return qobject_cast<KisLayer*>(node.data());
        }

    private:
        KisTransparencyMaskSP mask;

    };

    struct MergeDownInfo : public MergeDownInfoBase {
        MergeDownInfo(KisImageSP _image,
                      KisLayerSP _prevLayer,
                      KisLayerSP _currLayer)
            : MergeDownInfoBase(_image),
              prevLayer(_prevLayer),
              currLayer(_currLayer)
        {
            frames =
                fetchLayerFramesRecursive(prevLayer) |
                fetchLayerFramesRecursive(currLayer);

            pinnedToTimeline = prevLayer->isPinnedToTimeline() || currLayer->isPinnedToTimeline();

            const KisPaintLayer *paintLayer = qobject_cast<KisPaintLayer*>(currLayer.data());
            if (paintLayer) enableOnionSkins |= paintLayer->onionSkinEnabled();

            paintLayer = qobject_cast<KisPaintLayer*>(prevLayer.data());
            if (paintLayer) enableOnionSkins |= paintLayer->onionSkinEnabled();
        }

        KisLayerSP prevLayer;
        KisLayerSP currLayer;

        KisNodeList allSrcNodes() override {
            KisNodeList mergedNodes;
            mergedNodes << prevLayer;
            mergedNodes << currLayer;
            return mergedNodes;
        }
    };

    struct ConvertToPaintLayerInfo {
        ConvertToPaintLayerInfo(KisImageSP image, KisNodeSP node)
            : storage(new SwitchFrameCommand::SharedStorage())
            , m_sourceNode(node)
            , m_image(image)
            , m_putBehind(false)
            , m_pinnedToTimeline(false)
        {
            m_frames = fetchLayerFramesRecursive(node);

            m_pinnedToTimeline = node->isPinnedToTimeline();

            m_sourcePaintDevice =
                    m_sourceNode->paintDevice() ? m_sourceNode->projection() : m_sourceNode->original();

            m_compositeOp = m_sourceNode->projectionLeaf()->isLayer() ? m_sourceNode->compositeOpId() : COMPOSITE_OVER;

            KisColorizeMask *colorizeMask = dynamic_cast<KisColorizeMask*>(m_sourceNode.data());
            if (colorizeMask) {
                m_sourcePaintDevice = colorizeMask->coloringProjection();
                m_putBehind = colorizeMask->compositeOpId() == COMPOSITE_BEHIND;
                if (m_putBehind) {
                    m_compositeOp = COMPOSITE_OVER;
                }
            }

            if (m_sourcePaintDevice) {
                KisPaintDeviceSP clone;

                if (*m_sourcePaintDevice->colorSpace() !=
                        *m_sourcePaintDevice->compositionSourceColorSpace()) {

                    clone = new KisPaintDevice(m_sourcePaintDevice->compositionSourceColorSpace());
                    clone->setDefaultPixel(
                        m_sourcePaintDevice->defaultPixel().convertedTo(
                            m_sourcePaintDevice->compositionSourceColorSpace()));

                    QRect rc(m_sourcePaintDevice->extent());
                    KisPainter::copyAreaOptimized(rc.topLeft(), m_sourcePaintDevice, clone, rc);
                } else {
                    clone = new KisPaintDevice(*m_sourcePaintDevice);
                }

                m_targetNode = new KisPaintLayer(m_image,
                                                     m_sourceNode->name(),
                                                     m_sourceNode->opacity(),
                                                     clone);

                m_targetNode->setCompositeOpId(m_compositeOp);

                if (sourceLayer() && targetLayer()) {
                    targetLayer()->disableAlphaChannel(sourceLayer()->alphaChannelDisabled());
                }

                if (sourcePaintLayer() && targetPaintLayer()) {
                    targetPaintLayer()->setAlphaLocked(sourcePaintLayer()->alphaLocked());
                }
            }
        }

        QSet<int> frames() {
            return m_frames;
        }

        KisNodeSP sourceNode() {
            return m_sourceNode;
        }

        KisLayerSP sourceLayer() {
            return qobject_cast<KisLayer*>(m_sourceNode.data());
        }

        KisNodeList sourceNodes() {
            KisNodeList list;
            list << m_sourceNode;
            return list;
        }

        KisPaintLayerSP sourcePaintLayer() {
            return qobject_cast<KisPaintLayer*>(m_sourceNode.data());
        }

        bool hasTargetNode() {
            return m_targetNode != nullptr;
        }

        KisNodeSP targetNode() {
            return m_targetNode;
        }

        KisLayerSP targetLayer() {
            return qobject_cast<KisLayer*>(m_targetNode.data());
        }

        KisPaintLayerSP targetPaintLayer() {
            return qobject_cast<KisPaintLayer*>(m_targetNode.data());
        }


        KisImageSP image() {
            return m_image;
        }

        KisPaintDeviceSP paintDevice() {
            return m_sourcePaintDevice;
        }

        KisNodeList toRemove() {
            KisNodeList lst;
            lst << m_sourceNode;
            return lst;
        }

        SwitchFrameCommand::SharedStorageSP storage;

    private:
        KisNodeSP m_sourceNode;
        KisNodeSP m_targetNode;
        KisImageWSP m_image;
        KisPaintDeviceSP m_sourcePaintDevice;
        QSet<int> m_frames;
        bool m_putBehind;
        QString m_compositeOp;
        bool m_pinnedToTimeline;
    };

    struct MergeMultipleInfo : public MergeDownInfoBase {
        MergeMultipleInfo(KisImageSP _image,
                          KisNodeList _mergedNodes)
            : MergeDownInfoBase(_image),
              mergedNodes(_mergedNodes)
        {
            foreach (KisNodeSP node, mergedNodes) {
                frames |= fetchLayerFramesRecursive(node);
                pinnedToTimeline |= node->isPinnedToTimeline();

                const KisPaintLayer *paintLayer = qobject_cast<KisPaintLayer*>(node.data());
                if (paintLayer) {
                    enableOnionSkins |= paintLayer->onionSkinEnabled();
                }
            }
        }

        KisNodeList mergedNodes;
        bool nodesCompositingVaries = false;

        KisNodeList allSrcNodes() override {
            return mergedNodes;
        }
    };

    typedef QSharedPointer<MergeDownInfoBase> MergeDownInfoBaseSP;
    typedef QSharedPointer<MergeDownInfo> MergeDownInfoSP;
    typedef QSharedPointer<MergeMultipleInfo> MergeMultipleInfoSP;
    typedef QSharedPointer<SplitAlphaToMaskInfo> SplitAlphaToMaskInfoSP;
    typedef QSharedPointer<ConvertToPaintLayerInfo> ConvertToPaintLayerInfoSP;

    struct FillSelectionMasks : public KUndo2Command {
        FillSelectionMasks(MergeDownInfoBaseSP info) : m_info(info) {}

        void redo() override {
            fetchSelectionMasks(m_info->allSrcNodes(), m_info->selectionMasks);
        }

    private:
        MergeDownInfoBaseSP m_info;
    };

    struct DisableColorizeKeyStrokes : public KisCommandUtils::AggregateCommand {
        DisableColorizeKeyStrokes(MergeDownInfoBaseSP info) : m_info(info) {}

        void populateChildCommands() override {
            Q_FOREACH (KisNodeSP node, m_info->allSrcNodes()) {
                recursiveApplyNodes(node,
                                    [this] (KisNodeSP node) {
                                        if (dynamic_cast<KisColorizeMask*>(node.data()) &&
                                            KisLayerPropertiesIcons::nodeProperty(node, KisLayerPropertiesIcons::colorizeEditKeyStrokes, true).toBool()) {

                                            KisBaseNode::PropertyList props = node->sectionModelProperties();
                                            KisLayerPropertiesIcons::setNodeProperty(&props,
                                                                                     KisLayerPropertiesIcons::colorizeEditKeyStrokes,
                                                                                     false);

                                            addCommand(new KisNodePropertyListCommand(node, props));
                                        }
                                    });
            }
        }

    private:
        MergeDownInfoBaseSP m_info;
    };

    struct DisableOnionSkins : public KisCommandUtils::AggregateCommand {
        DisableOnionSkins(MergeDownInfoBaseSP info) : m_info(info) {}

        void populateChildCommands() override {
            Q_FOREACH (KisNodeSP node, m_info->allSrcNodes()) {
                recursiveApplyNodes(node,
                                    [this] (KisNodeSP node) {
                                        if (KisLayerPropertiesIcons::nodeProperty(node, KisLayerPropertiesIcons::onionSkins, false).toBool()) {

                                            KisBaseNode::PropertyList props = node->sectionModelProperties();
                                            KisLayerPropertiesIcons::setNodeProperty(&props,
                                                                                     KisLayerPropertiesIcons::onionSkins,
                                                                                     false);

                                            addCommand(new KisNodePropertyListCommand(node, props));
                                        }
                                    });
            }
        }

    private:
        MergeDownInfoBaseSP m_info;
    };

    struct DisableExtraCompositing : public KisCommandUtils::AggregateCommand {
        DisableExtraCompositing(MergeMultipleInfoSP info) : m_info(info) {}

        void populateChildCommands() override {
            /**
             * We disable extra compositing only in case all the layers have
             * the same compositing properties, therefore, we can just sum them using
             * Normal blend mode
             */
            if (m_info->nodesCompositingVaries) return;

            // we should disable dirty requests on **redo only**, otherwise
            // the state of the layers will not be recovered on undo
            m_info->image->disableDirtyRequests();

            Q_FOREACH (KisNodeSP node, m_info->allSrcNodes()) {
                if (node->compositeOpId() != COMPOSITE_OVER) {
                    addCommand(new KisNodeCompositeOpCommand(node, node->compositeOpId(), COMPOSITE_OVER));
                }

                if (KisLayerPropertiesIcons::nodeProperty(node, KisLayerPropertiesIcons::inheritAlpha, false).toBool()) {

                    KisBaseNode::PropertyList props = node->sectionModelProperties();
                    KisLayerPropertiesIcons::setNodeProperty(&props,
                                                             KisLayerPropertiesIcons::inheritAlpha,
                                                             false);

                    addCommand(new KisNodePropertyListCommand(node, props));
                }
            }

            m_info->image->enableDirtyRequests();
        }

    private:
        MergeMultipleInfoSP m_info;
    };

    struct DisablePassThroughForHeadsOnly : public KisCommandUtils::AggregateCommand {
        DisablePassThroughForHeadsOnly(MergeDownInfoBaseSP info, bool skipIfDstIsGroup = false)
            : m_info(info),
              m_skipIfDstIsGroup(skipIfDstIsGroup)
        {
        }

        void populateChildCommands() override {
            if (m_skipIfDstIsGroup &&
                m_info->dstLayer() &&
                m_info->dstLayer()->inherits("KisGroupLayer")) {

                return;
            }


            Q_FOREACH (KisNodeSP node, m_info->allSrcNodes()) {
                if (KisLayerPropertiesIcons::nodeProperty(node, KisLayerPropertiesIcons::passThrough, false).toBool()) {

                    KisBaseNode::PropertyList props = node->sectionModelProperties();
                    KisLayerPropertiesIcons::setNodeProperty(&props,
                                                             KisLayerPropertiesIcons::passThrough,
                                                             false);

                    addCommand(new KisNodePropertyListCommand(node, props));
                }
            }
        }

    private:
        MergeDownInfoBaseSP m_info;
        bool m_skipIfDstIsGroup;
    };

    struct RefreshHiddenAreas : public KUndo2Command {
        RefreshHiddenAreas(MergeDownInfoBaseSP info) : m_image(info->image), m_nodes(info->allSrcNodes()) {}
        RefreshHiddenAreas(KisImageSP image, KisNodeList nodes) : m_image(image), m_nodes(nodes) {}
        RefreshHiddenAreas(KisImageSP image, KisNodeSP node) : m_image(image), m_nodes() {
            m_nodes << node;
        }

        void redo() override {
            KisImageAnimationInterface *interface = m_image->animationInterface();
            const QRect preparedRect = !interface->externalFrameActive() ?
                m_image->bounds() : QRect();

            foreach (KisNodeSP node, m_nodes) {
                refreshHiddenAreaAsync(m_image, node, preparedRect);
            }
        }

    private:
        KisImageWSP m_image;
        KisNodeList m_nodes;
    };

    struct RefreshDelayedUpdateLayers : public KUndo2Command {
        RefreshDelayedUpdateLayers(MergeDownInfoBaseSP info)
            : m_nodes(info->allSrcNodes()) {}

        RefreshDelayedUpdateLayers(KisNodeList nodes){
            m_nodes << nodes;
        }

        void redo() override {
            if (m_info) {
                m_nodes << m_info->allSrcNodes();
            }

            foreach (KisNodeSP node, m_nodes) {
                forceAllDelayedNodesUpdate(node);
            }
        }

    private:
        KisNodeList m_nodes;
        MergeDownInfoBaseSP m_info;
    };

    struct KeepMergedNodesSelected : public KisCommandUtils::AggregateCommand {
        KeepMergedNodesSelected(MergeDownInfoSP info, bool finalizing)
            : m_singleInfo(info),
              m_finalizing(finalizing) {}

        KeepMergedNodesSelected(MergeMultipleInfoSP info, KisNodeSP putAfter, bool finalizing)
            : m_multipleInfo(info),
              m_finalizing(finalizing),
              m_putAfter(putAfter) {}

        void populateChildCommands() override {
            KisNodeSP prevNode;
            KisNodeSP nextNode;
            KisNodeList prevSelection;
            KisNodeList nextSelection;
            KisImageSP image;

            if (m_singleInfo) {
                prevNode = m_singleInfo->currLayer;
                nextNode = m_singleInfo->dstNode;
                image = m_singleInfo->image;
            } else if (m_multipleInfo) {
                prevNode = m_putAfter;
                nextNode = m_multipleInfo->dstNode;
                prevSelection = m_multipleInfo->allSrcNodes();
                image = m_multipleInfo->image;
            }

            if (!m_finalizing) {
                addCommand(new KeepNodesSelectedCommand(prevSelection, KisNodeList(),
                                                        prevNode, KisNodeSP(),
                                                        image, false));
            } else {
                addCommand(new KeepNodesSelectedCommand(KisNodeList(), nextSelection,
                                                        KisNodeSP(), nextNode,
                                                        image, true));
            }
        }

    private:
        MergeDownInfoSP m_singleInfo;
        MergeMultipleInfoSP m_multipleInfo;
        bool m_finalizing;
        KisNodeSP m_putAfter;
    };

    struct CreateMergedLayer : public KisCommandUtils::AggregateCommand {
        CreateMergedLayer(MergeDownInfoSP info) : m_info(info) {}

        void populateChildCommands() override {
            // actual merging done by KisLayer::createMergedLayer (or specialized descendant)
            m_info->dstNode = m_info->currLayer->createMergedLayerTemplate(m_info->prevLayer);

            if (m_info->frames.size() > 0) {
                m_info->dstNode->enableAnimation();
                m_info->dstNode->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
            }

            m_info->dstNode->setPinnedToTimeline(m_info->pinnedToTimeline);
            m_info->dstNode->setColorLabelIndex(m_info->allSrcNodes().first()->colorLabelIndex());

            KisPaintLayer *dstPaintLayer = qobject_cast<KisPaintLayer*>(m_info->dstNode.data());
            if (dstPaintLayer) {
                dstPaintLayer->setOnionSkinEnabled(m_info->enableOnionSkins);
            }
        }

    private:
        MergeDownInfoSP m_info;
    };

    struct CreateMergedLayerMultiple : public KisCommandUtils::AggregateCommand {
        CreateMergedLayerMultiple(MergeMultipleInfoSP info, const QString name = QString() )
            : m_info(info),
              m_name(name) {}

        void populateChildCommands() override {
            QString mergedLayerName;
            if (m_name.isEmpty()){
                const QString mergedLayerSuffix = i18n("Merged");
                mergedLayerName = m_info->mergedNodes.first()->name();

                if (!mergedLayerName.endsWith(mergedLayerSuffix)) {
                    mergedLayerName = QString("%1 %2")
                        .arg(mergedLayerName).arg(mergedLayerSuffix);
                }
            } else {
                mergedLayerName = m_name;
            }

            KisPaintLayer *dstPaintLayer = new KisPaintLayer(m_info->image, mergedLayerName, OPACITY_OPAQUE_U8);
            m_info->dstNode = dstPaintLayer;

            if (m_info->frames.size() > 0) {
                m_info->dstNode->enableAnimation();
                m_info->dstNode->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
            }


            auto channelFlagsLazy = [](KisNodeSP node) {
                KisLayer *layer = dynamic_cast<KisLayer*>(node.data());
                return layer ? layer->channelFlags() : QBitArray();
            };

            QString compositeOpId;
            QBitArray channelFlags;
            bool compositionVaries = false;
            bool isFirstCycle = true;

            foreach (KisNodeSP node, m_info->allSrcNodes()) {
                if (isFirstCycle) {
                    compositeOpId = node->compositeOpId();
                    channelFlags = channelFlagsLazy(node);
                    isFirstCycle = false;
                } else if (compositeOpId != node->compositeOpId() ||
                           channelFlags != channelFlagsLazy(node)) {
                    compositionVaries = true;
                    break;
                }

                KisLayerSP layer = qobject_cast<KisLayer*>(node.data());
                if (layer && layer->layerStyle()) {
                    compositionVaries = true;
                    break;
                }
            }

            if (!compositionVaries) {
                if (!compositeOpId.isEmpty()) {
                    m_info->dstNode->setCompositeOpId(compositeOpId);
                }
                if (m_info->dstLayer() && !channelFlags.isEmpty()) {
                    m_info->dstLayer()->setChannelFlags(channelFlags);
                }
            }

            m_info->nodesCompositingVaries = compositionVaries;

            m_info->dstNode->setPinnedToTimeline(m_info->pinnedToTimeline);
            m_info->dstNode->setColorLabelIndex(m_info->allSrcNodes().first()->colorLabelIndex());

            dstPaintLayer->setOnionSkinEnabled(m_info->enableOnionSkins);
        }

    private:
        MergeMultipleInfoSP m_info;
        QString m_name;
    };

    struct MergeLayers : public KisCommandUtils::AggregateCommand {
        MergeLayers(MergeDownInfoSP info) : m_info(info) {}

        void populateChildCommands() override {
            // actual merging done by KisLayer::createMergedLayer (or specialized descendant)
            m_info->currLayer->fillMergedLayerTemplate(m_info->dstLayer(), m_info->prevLayer);
        }

    private:
        MergeDownInfoSP m_info;
    };

    struct MergeLayersMultiple : public KisCommandUtils::AggregateCommand {
        MergeLayersMultiple(MergeMultipleInfoSP info) : m_info(info) {}

        void populateChildCommands() override {
            KisPainter gc(m_info->dstNode->paintDevice());

            foreach (KisNodeSP node, m_info->allSrcNodes()) {
                QRect rc = node->exactBounds() | m_info->image->bounds();
                node->projectionPlane()->apply(&gc, rc);
            }
        }

    private:
        MergeMultipleInfoSP m_info;
    };

    struct MergeMetaData : public KUndo2Command {
        MergeMetaData(MergeDownInfoSP info, const KisMetaData::MergeStrategy* strategy)
            : m_info(info),
              m_strategy(strategy) {}

        void redo() override {
            QRect layerProjectionExtent = m_info->currLayer->projection()->extent();
            QRect prevLayerProjectionExtent = m_info->prevLayer->projection()->extent();
            int prevLayerArea = prevLayerProjectionExtent.width() * prevLayerProjectionExtent.height();
            int layerArea = layerProjectionExtent.width() * layerProjectionExtent.height();

            QList<double> scores;
            double norm = qMax(prevLayerArea, layerArea);
            scores.append(prevLayerArea / norm);
            scores.append(layerArea / norm);

            QList<const KisMetaData::Store*> srcs;
            srcs.append(m_info->prevLayer->metaData());
            srcs.append(m_info->currLayer->metaData());
            m_strategy->merge(m_info->dstLayer()->metaData(), srcs, scores);
        }

    private:
        MergeDownInfoSP m_info;
        const KisMetaData::MergeStrategy *m_strategy;
    };

    struct InitSplitAlphaSelectionMask : public KisCommandUtils::AggregateCommand  {
        InitSplitAlphaSelectionMask(SplitAlphaToMaskInfoSP info)
            : m_info(info) {}

        void populateChildCommands() override {
            m_info->getMask()->initSelection(m_info->getLayer());
        }

    private:
        SplitAlphaToMaskInfoSP m_info;
    };

    struct SplitAlphaCommand : public KUndo2Command  {
        SplitAlphaCommand(SplitAlphaToMaskInfoSP info)
            : m_info(info) {
            m_cached = new KisPaintDevice(*m_info->node->paintDevice(), KritaUtils::CopyAllFrames);
        }

        void redo() override {
            KisPaintDeviceSP srcDevice = m_info->node->paintDevice();
            const KoColorSpace *srcCS = srcDevice->colorSpace();
            const QRect processRect =
                    srcDevice->exactBounds() |
                    srcDevice->defaultBounds()->bounds();

            KisSequentialIterator srcIt(srcDevice, processRect);
            KisSequentialIterator dstIt(m_info->getMaskDevice(), processRect);

            while (srcIt.nextPixel() && dstIt.nextPixel()) {
                quint8 *srcPtr = srcIt.rawData();
                quint8 *alpha8Ptr = dstIt.rawData();

                *alpha8Ptr = srcCS->opacityU8(srcPtr);
                srcCS->setOpacity(srcPtr, OPACITY_OPAQUE_U8, 1);
            }
        }

        void undo() override {
            KisPaintDeviceSP srcDevice = m_info->node->paintDevice();

            if (srcDevice->framesInterface()) { //Swap contents of all frames to reflect the pre-operation state.
                KisPaintDeviceSP tempPD = new KisPaintDevice(*m_cached, KritaUtils::CopySnapshot);
                Q_FOREACH(const int& frame, srcDevice->framesInterface()->frames() ) {
                    if (m_cached->framesInterface()->frames().contains(frame)) {
                        m_cached->framesInterface()->writeFrameToDevice(frame, tempPD);
                        srcDevice->framesInterface()->uploadFrame(frame, tempPD);
                    }
                }
            } else {
                const QRect processRect =
                        srcDevice->exactBounds() |
                        srcDevice->defaultBounds()->bounds();

                const KoColorSpace *srcCS = srcDevice->colorSpace();
                KisSequentialIterator srcIt(m_cached, processRect);
                KisSequentialIterator dstIt(srcDevice, processRect);

                while (srcIt.nextPixel() && dstIt.nextPixel()) {
                    quint8 *srcPtr = srcIt.rawData();
                    quint8 *dstPtr = dstIt.rawData();
                    srcCS->setOpacity(dstPtr, srcCS->opacityU8(srcPtr), 1);
                }
            }
        }

    private:
        SplitAlphaToMaskInfoSP m_info;
        KisPaintDeviceSP m_cached;
    };

    struct UploadProjectionToFrameCommand : public KisCommandUtils::AggregateCommand {
        UploadProjectionToFrameCommand(KisNodeSP src, KisNodeSP target, int frame)
            : m_source(src)
            , m_target(target)
            , m_frame(frame)
        {}

        void populateChildCommands() override {
            KisRasterKeyframeChannel* channel = dynamic_cast<KisRasterKeyframeChannel*>(m_target->getKeyframeChannel(KisKeyframeChannel::Raster.id()));
            if (!channel)
                return;


            KisPaintDeviceSP clone = new KisPaintDevice(*m_source->projection());
            KisRasterKeyframeSP key = channel->keyframeAt<KisRasterKeyframe>(m_frame);
            m_target->paintDevice()->framesInterface()->uploadFrame(key->frameID(), clone);
        }

    private:
        KisNodeSP m_source;
        KisNodeSP m_target;
        int m_frame;
    };

    KeepNodesSelectedCommand::KeepNodesSelectedCommand(const KisNodeList &selectedBefore,
                                                       const KisNodeList &selectedAfter,
                                                       KisNodeSP activeBefore,
                                                       KisNodeSP activeAfter,
                                                       KisImageSP image,
                                                       bool finalize, KUndo2Command *parent)
        : FlipFlopCommand(finalize, parent),
          m_selectedBefore(selectedBefore),
          m_selectedAfter(selectedAfter),
          m_activeBefore(activeBefore),
          m_activeAfter(activeAfter),
          m_image(image)
    {
    }

    void KeepNodesSelectedCommand::partB() {
        KisImageSignalType type;
        if (getState() == State::FINALIZING) {
            type = ComplexNodeReselectionSignal(m_activeAfter, m_selectedAfter);
        } else {
            type = ComplexNodeReselectionSignal(m_activeBefore, m_selectedBefore);
        }
        m_image->signalRouter()->emitNotification(type);
    }

    SelectGlobalSelectionMask::SelectGlobalSelectionMask(KisImageSP image)
        : m_image(image)
    {
    }

    void SelectGlobalSelectionMask::redo() {

        KisImageSignalType type =
                ComplexNodeReselectionSignal(m_image->rootLayer()->selectionMask(), KisNodeList());
        m_image->signalRouter()->emitNotification(type);

    }

    RemoveNodeHelper::~RemoveNodeHelper()
    {
    }

    /**
     * The removal of two nodes in one go may be a bit tricky, because one
     * of them may be the clone of another. If we remove the source of a
     * clone layer, it will reincarnate into a paint layer. In this case
     * the pointer to the second layer will be lost.
     *
     * That's why we need to care about the order of the nodes removal:
     * the clone --- first, the source --- last.
     */
    void RemoveNodeHelper::safeRemoveMultipleNodes(KisNodeList nodes, KisImageSP image) {
        const bool lastLayer = scanForLastLayer(image, nodes);

        auto isNodeWeird = [] (KisNodeSP node) {
            const bool normalCompositeMode = node->compositeOpId() == COMPOSITE_OVER;

            KisLayer *layer = dynamic_cast<KisLayer*>(node.data());
            const bool hasInheritAlpha = layer && layer->alphaChannelDisabled();
            return !normalCompositeMode && !hasInheritAlpha;
        };

        while (!nodes.isEmpty()) {
            KisNodeList::iterator it = nodes.begin();

            while (it != nodes.end()) {
                if (!checkIsSourceForClone(*it, nodes)) {
                    KisNodeSP node = *it;

                    addCommandImpl(new KisImageLayerRemoveCommand(image, node, !isNodeWeird(node), true));
                    it = nodes.erase(it);
                } else {
                    ++it;
                }
            }
        }

        if (lastLayer) {
            KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, image->colorSpace());
            addCommandImpl(new KisImageLayerAddCommand(image, newLayer,
                                                       image->root(),
                                                       KisNodeSP(),
                                                       false, false));
        }
    }

    bool RemoveNodeHelper::checkIsSourceForClone(KisNodeSP src, const KisNodeList &nodes) {
        foreach (KisNodeSP node, nodes) {
            if (node == src) continue;

            KisCloneLayer *clone = dynamic_cast<KisCloneLayer*>(node.data());

            if (clone && KisNodeSP(clone->copyFrom()) == src) {
                return true;
            }
        }

        return false;
    }

    bool RemoveNodeHelper::scanForLastLayer(KisImageWSP image, KisNodeList nodesToRemove) {
        bool removeLayers = false;
        Q_FOREACH(KisNodeSP nodeToRemove, nodesToRemove) {
            if (qobject_cast<KisLayer*>(nodeToRemove.data())) {
                removeLayers = true;
                break;
            }
        }
        if (!removeLayers) return false;

        bool lastLayer = true;
        KisNodeSP node = image->root()->firstChild();
        while (node) {
            if (!nodesToRemove.contains(node) &&
                qobject_cast<KisLayer*>(node.data()) &&
                !node->isFakeNode()) {

                lastLayer = false;
                break;
            }
            node = node->nextSibling();
        }

        return lastLayer;
    }

    SimpleRemoveLayers::SimpleRemoveLayers(const KisNodeList &nodes,
                                           KisImageSP image)
        : m_nodes(nodes),
          m_image(image)
    {
    }

    void SimpleRemoveLayers::populateChildCommands() {
        if (m_nodes.isEmpty()) return;
        safeRemoveMultipleNodes(m_nodes, m_image);
    }

    void SimpleRemoveLayers::addCommandImpl(KUndo2Command *cmd) {
        addCommand(cmd);
    }

    struct InsertNode : public KisCommandUtils::AggregateCommand {
        InsertNode(MergeDownInfoBaseSP info, KisNodeSP putAfter)
            : m_info(info), m_putAfter(putAfter) {}

        void populateChildCommands() override {
            addCommand(new KisImageLayerAddCommand(m_info->image,
                                                           m_info->dstNode,
                                                           m_putAfter->parent(),
                                                           m_putAfter,
                                                           true, false));

        }

    private:
        virtual void addCommandImpl(KUndo2Command *cmd) {
            addCommand(cmd);
        }

        MergeDownInfoBaseSP m_info;
        KisNodeSP m_putAfter;
    };

    struct SimpleAddNode : public KisCommandUtils::AggregateCommand {
        SimpleAddNode(KisImageSP image, KisNodeSP toAdd, KisNodeSP parent = 0, KisNodeSP putAfter = 0)
            : m_image(image)
            , m_toAdd(toAdd)
            , m_parent(parent)
            , m_putAfter(putAfter)
        {
            while (m_parent && !m_parent->allowAsChild(m_toAdd)) {
                m_putAfter = m_putAfter ? m_putAfter->parent() : m_parent;
                m_parent = m_putAfter ? m_putAfter->parent() : 0;
            }

            if (!m_parent) {
                m_parent = m_image->root();
            }
        }


        void populateChildCommands() override {
            addCommand(new KisImageLayerAddCommand(m_image,
                                                       m_toAdd,
                                                       m_parent,
                                                       m_putAfter,
                                                       true, false));
        }

    private:
        virtual void addCommandImpl(KUndo2Command *cmd) {
            addCommand(cmd);
        }

        KisImageWSP m_image;
        KisNodeSP m_toAdd;
        KisNodeSP m_parent;
        KisNodeSP m_putAfter;

    };


    void splitNonRemovableNodes(KisNodeList &nodesToRemove, KisNodeList &_nodesToHide)
    {
        QSet<KisNodeSP> nodesToHide;
        QSet<KisNodeSP> extraNodesToRemove;

        for (auto it = nodesToRemove.begin(); it != nodesToRemove.end(); ++it) {
            KisNodeSP root = *it;
            KIS_SAFE_ASSERT_RECOVER_NOOP(root->visible());

            if (!root->isEditable(false)) {
                nodesToHide.insert(root);
            } else {
                bool rootNeedsCarefulRemoval = false;

                recursiveApplyNodes(root,
                                    [root, &nodesToHide, &rootNeedsCarefulRemoval] (KisNodeSP node) {
                                        if (!node->isEditable(false)) {
                                            while (node != root) {
                                                nodesToHide.insert(node);
                                                node = node->parent();
                                                KIS_SAFE_ASSERT_RECOVER_BREAK(node);
                                            }
                                            nodesToHide.insert(root);
                                            rootNeedsCarefulRemoval = true;
                                        }
                                    });

                if (rootNeedsCarefulRemoval) {
                    recursiveApplyNodes(root,
                                        [&extraNodesToRemove] (KisNodeSP node) {
                                            extraNodesToRemove.insert(node);
                                        });
                }
            }
        }
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        nodesToRemove += KisNodeList(extraNodesToRemove.begin(), extraNodesToRemove.end());
#else
        nodesToRemove += extraNodesToRemove.toList();
#endif
        KritaUtils::filterContainer<KisNodeList>(nodesToRemove,
                                                 [nodesToHide](KisNodeSP node) {
                                                     return !nodesToHide.contains(node);
                                                 });
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        _nodesToHide = KisNodeList(nodesToHide.begin(), nodesToHide.end());
#else
        _nodesToHide = nodesToHide.toList();
#endif
    }

    struct CleanUpNodes : private RemoveNodeHelper, public KisCommandUtils::AggregateCommand {
        CleanUpNodes(MergeDownInfoBaseSP info, KisNodeSP putAfter)
            : m_info(info), m_putAfter(putAfter) {}

        static void findPerfectParent(KisNodeList nodesToDelete, KisNodeSP &putAfter, KisNodeSP &parent) {
            if (!putAfter) {
                putAfter = nodesToDelete.last();
            }

            // Add the new merged node on top of the active node
            // -- checking all parents if they are included in nodesToDelete
            // Not every descendant is included in nodesToDelete even if in fact
            //   they are going to be deleted, so we need to check it.
            // If we consider the path from root to the putAfter node,
            //    if there are any nodes marked for deletion, any node afterwards
            //    is going to be deleted, too.
            //   example:      root . . . . . ! ! . . ! ! ! ! . . . . putAfter
            //   it should be: root . . . . . ! ! ! ! ! ! ! ! ! ! ! ! !putAfter
            //   and here:     root . . . . X ! ! . . ! ! ! ! . . . . putAfter
            //   you can see which node is "the perfect ancestor"
            //   (marked X; called "parent" in the function arguments).
            //   and here:     root . . . . . O ! . . ! ! ! ! . . . . putAfter
            //   you can see which node is "the topmost deleted ancestor" (marked 'O')

            KisNodeSP node = putAfter->parent();
            bool foundDeletedAncestor = false;
            KisNodeSP topmostAncestorToDelete = nullptr;

            while (node) {

                if (nodesToDelete.contains(node)
                        && !nodesToDelete.contains(node->parent())) {
                    foundDeletedAncestor = true;
                    topmostAncestorToDelete = node;
                    // Here node is to be deleted and its parent is not,
                    // so its parent is the one of the first not deleted (="perfect") ancestors.
                    // We need the one that is closest to the top (root)
                }

                node = node->parent();
            }

            if (foundDeletedAncestor) {
                parent = topmostAncestorToDelete->parent();
                putAfter = topmostAncestorToDelete;
            }
            else {
                parent = putAfter->parent(); // putAfter (and none of its ancestors) is to be deleted, so its parent is the first not deleted ancestor
            }

        }

        void populateChildCommands() override {
            KisNodeList nodesToDelete = m_info->allSrcNodes();

            KisNodeSP parent;
            findPerfectParent(nodesToDelete, m_putAfter, parent);

            if (!parent) {
                KisNodeSP oldRoot = m_info->image->root();
                KisNodeSP newRoot(new KisGroupLayer(m_info->image, "root", OPACITY_OPAQUE_U8));

                // copy all fake nodes into the new image
                KisLayerUtils::recursiveApplyNodes(oldRoot, [this, oldRoot, newRoot] (KisNodeSP node) {
                    if (node->isFakeNode() && node->parent() == oldRoot) {
                        addCommand(new KisImageLayerAddCommand(m_info->image,
                                                               node->clone(),
                                                               newRoot,
                                                               KisNodeSP(),
                                                               false, false));

                    }
                });

                addCommand(new KisImageLayerAddCommand(m_info->image,
                                                       m_info->dstNode,
                                                       newRoot,
                                                       KisNodeSP(),
                                                       true, false));
                addCommand(new KisImageChangeLayersCommand(m_info->image, oldRoot, newRoot));

            }
            else {
                addCommand(new KisImageLayerAddCommand(m_info->image,
                                                       m_info->dstNode,
                                                       parent,
                                                       m_putAfter,
                                                       true, false));


                /**
                 * We can merge selection masks, in this case dstLayer is not defined!
                 */
                if (m_info->dstLayer()) {
                    reparentSelectionMasks(m_info->image,
                                           m_info->dstLayer(),
                                           m_info->selectionMasks);
                }

                KisNodeList safeNodesToDelete = m_info->allSrcNodes();
                KisNodeList safeNodesToHide;

                splitNonRemovableNodes(safeNodesToDelete, safeNodesToHide);

                Q_FOREACH(KisNodeSP node, safeNodesToHide) {
                    addCommand(new KisImageChangeVisibilityCommand(false, node));
                }

                safeRemoveMultipleNodes(safeNodesToDelete, m_info->image);
            }


        }

    private:
        void addCommandImpl(KUndo2Command *cmd) override {
            addCommand(cmd);
        }

        void reparentSelectionMasks(KisImageSP image,
                                    KisLayerSP newLayer,
                                    const QVector<KisSelectionMaskSP> &selectionMasks) {

            KIS_SAFE_ASSERT_RECOVER_RETURN(newLayer);

            foreach (KisSelectionMaskSP mask, selectionMasks) {
                addCommand(new KisImageLayerMoveCommand(image, mask, newLayer, newLayer->lastChild()));
                addCommand(new KisActivateSelectionMaskCommand(mask, false));
            }
        }
    private:
        MergeDownInfoBaseSP m_info;
        KisNodeSP m_putAfter;
    };

    SwitchFrameCommand::SharedStorage::~SharedStorage() {
    }

    SwitchFrameCommand::SwitchFrameCommand(KisImageSP image, int time, bool finalize, SharedStorageSP storage)
        : FlipFlopCommand(finalize),
          m_image(image),
          m_newTime(time),
          m_storage(storage) {}

    SwitchFrameCommand::~SwitchFrameCommand() {}

    void SwitchFrameCommand::partA() {
        KisImageAnimationInterface *interface = m_image->animationInterface();
        const int currentTime = interface->currentTime();
        if (currentTime == m_newTime) {
            m_storage->value = m_newTime;
            return;
        }

        interface->image()->disableUIUpdates();
        interface->saveAndResetCurrentTime(m_newTime, &m_storage->value);
    }

    void SwitchFrameCommand::partB() {
        KisImageAnimationInterface *interface = m_image->animationInterface();
        const int currentTime = interface->currentTime();
        if (currentTime == m_storage->value) {
            return;
        }

        interface->restoreCurrentTime(&m_storage->value);
        interface->image()->enableUIUpdates();
    }

    struct AddNewFrame : public KisCommandUtils::AggregateCommand {
        AddNewFrame(KisNodeSP node, int frame) : m_node(node), m_frame(frame) {}
        AddNewFrame(KisNodeSP node, int frame, KisNodeList sampleNodes) : m_node(node), m_frame(frame), m_sampledNodes(sampleNodes) {}
        AddNewFrame(KisNodeSP node, int frame, KisNodeSP source) : m_node(node), m_frame(frame) { m_sampledNodes << source; }
        AddNewFrame(MergeDownInfoBaseSP info, int frame) : m_frame(frame), m_sampledNodes(info->allSrcNodes()), m_mergeInfo(info) {}

        void populateChildCommands() override {
            KUndo2Command *cmd = new KisCommandUtils::SkipFirstRedoWrapper();
            KisNodeSP node = m_node ? m_node : m_mergeInfo->dstNode;
            KisKeyframeChannel *channel = node->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
            channel->addKeyframe(m_frame, cmd);

            if (m_sampledNodes.count() > 0) {
                applyKeyframeColorLabel(channel->keyframeAt(m_frame), m_sampledNodes);
            }

            addCommand(cmd);
        }

        void applyKeyframeColorLabel(KisKeyframeSP dstKeyframe, KisNodeList srcNodes) {
            Q_FOREACH(KisNodeSP srcNode, srcNodes) {
                Q_FOREACH(KisKeyframeChannel *channel, srcNode->keyframeChannels().values()) {
                    KisKeyframeSP keyframe = channel->keyframeAt(m_frame);
                    if (!keyframe.isNull() && keyframe->colorLabel() != 0) {
                        dstKeyframe->setColorLabel(keyframe->colorLabel());
                        return;
                    }
                }
            }

            dstKeyframe->setColorLabel(0);
        }

    private:
        KisNodeSP m_node;
        int m_frame;
        KisNodeList m_sampledNodes;
        MergeDownInfoBaseSP m_mergeInfo;
    };

    QSet<int> fetchLayerFrames(KisNodeSP node) {
        QSet<int> frames;
        Q_FOREACH(KisKeyframeChannel *channel, node->keyframeChannels()) {
            if (!channel) {
                continue;
            }

            KisRasterKeyframeChannel *rasterChan = dynamic_cast<KisRasterKeyframeChannel*>(channel);
            if (rasterChan) {
                frames.unite(channel->allKeyframeTimes());
                continue;
            }

            KisScalarKeyframeChannel *scalarChan = dynamic_cast<KisScalarKeyframeChannel*>(channel);
            if (scalarChan) {
                const int initialKeyframe = scalarChan->firstKeyframeTime();
                const int lastKeyframe = scalarChan->lastKeyframeTime();
                KisTimeSpan currentSpan = scalarChan->identicalFrames(initialKeyframe);
                while (!currentSpan.isInfinite() && currentSpan.isValid() && currentSpan.start() < lastKeyframe) {
                    frames.insert(currentSpan.start());
                    currentSpan = scalarChan->identicalFrames(currentSpan.end() + 1);
                }

                frames.insert(lastKeyframe);
            }

        }

        return frames;
    }

    QSet<int> fetchLayerFramesRecursive(KisNodeSP rootNode) {
        if (!rootNode->visible()) return QSet<int>();

        QSet<int> frames = fetchLayerFrames(rootNode);

        KisNodeSP node = rootNode->firstChild();
        while(node) {
            frames |= fetchLayerFramesRecursive(node);
            node = node->nextSibling();
        }

        return frames;
    }

    void updateFrameJobs(FrameJobs *jobs, KisNodeSP node) {
        QSet<int> frames = fetchLayerFrames(node);

        if (frames.isEmpty()) {
            (*jobs)[0].insert(node);
        } else {
            foreach (int frame, frames) {
                (*jobs)[frame].insert(node);
            }
        }
    }

    void updateFrameJobsRecursive(FrameJobs *jobs, KisNodeSP rootNode) {
        updateFrameJobs(jobs, rootNode);

        KisNodeSP node = rootNode->firstChild();
        while(node) {
            updateFrameJobsRecursive(jobs, node);
            node = node->nextSibling();
        }
    }

    /**
     * \see a comment in mergeMultipleLayersImpl()
     */
    void mergeDown(KisImageSP image, KisLayerSP layer, const KisMetaData::MergeStrategy* strategy)
    {
        if (!layer->prevSibling()) return;

        // XXX: this breaks if we allow free mixing of masks and layers
        KisLayerSP prevLayer = qobject_cast<KisLayer*>(layer->prevSibling().data());
        if (!prevLayer) return;

        if (!layer->visible() && !prevLayer->visible()) {
            return;
        }

        KisImageSignalVector emitSignals;
        KisProcessingApplicator applicator(image, 0,
                                           KisProcessingApplicator::NONE,
                                           emitSignals,
                                           kundo2_i18n("Merge Down"));

        if (layer->visible() && prevLayer->visible()) {
            MergeDownInfoSP info(new MergeDownInfo(image, prevLayer, layer));

            // disable key strokes on all colorize masks, all onion skins on
            // paint layers and wait until update is finished with a barrier
            applicator.applyCommand(new DisableColorizeKeyStrokes(info));
            applicator.applyCommand(new DisableOnionSkins(info));
            applicator.applyCommand(new KUndo2Command(), KisStrokeJobData::BARRIER);

            applicator.applyCommand(new KeepMergedNodesSelected(info, false));
            applicator.applyCommand(new FillSelectionMasks(info));
            applicator.applyCommand(new CreateMergedLayer(info), KisStrokeJobData::BARRIER);

            // NOTE: shape layer may have emitted spontaneous jobs during layer creation,
            //       wait for them to complete!
            applicator.applyCommand(new RefreshDelayedUpdateLayers(info), KisStrokeJobData::BARRIER);
            applicator.applyCommand(new KUndo2Command(), KisStrokeJobData::BARRIER);

            // in two-layer mode we disable pass through only when the destination layer
            // is not a group layer
            applicator.applyCommand(new DisablePassThroughForHeadsOnly(info, true));
            applicator.applyCommand(new KUndo2Command(), KisStrokeJobData::BARRIER);

            if (info->frames.size() > 0) {
                foreach (int frame, info->frames) {
                    applicator.applyCommand(new SwitchFrameCommand(info->image, frame, false, info->storage));

                    applicator.applyCommand(new AddNewFrame(info, frame));

                    applicator.applyCommand(new RefreshHiddenAreas(info));
                    applicator.applyCommand(new RefreshDelayedUpdateLayers(info), KisStrokeJobData::BARRIER);

                    applicator.applyCommand(new MergeLayers(info), KisStrokeJobData::BARRIER);

                    applicator.applyCommand(new SwitchFrameCommand(info->image, frame, true, info->storage), KisStrokeJobData::BARRIER);
                }
            } else {
                applicator.applyCommand(new RefreshHiddenAreas(info));
                applicator.applyCommand(new RefreshDelayedUpdateLayers(info), KisStrokeJobData::BARRIER);
                applicator.applyCommand(new MergeLayers(info), KisStrokeJobData::BARRIER);
            }

            applicator.applyCommand(new MergeMetaData(info, strategy), KisStrokeJobData::BARRIER);
            applicator.applyCommand(new CleanUpNodes(info, layer),
                                    KisStrokeJobData::SEQUENTIAL,
                                    KisStrokeJobData::EXCLUSIVE);
            applicator.applyCommand(new KeepMergedNodesSelected(info, true));
        } else if (layer->visible()) {
            applicator.applyCommand(new KeepNodesSelectedCommand(KisNodeList(), KisNodeList(),
                                                                 layer, KisNodeSP(),
                                                                 image, false));

            applicator.applyCommand(
                new SimpleRemoveLayers(KisNodeList() << prevLayer,
                                       image),
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE);

            applicator.applyCommand(new KeepNodesSelectedCommand(KisNodeList(), KisNodeList(),
                                                                 KisNodeSP(), layer,
                                                                 image, true));
        } else if (prevLayer->visible()) {
            applicator.applyCommand(new KeepNodesSelectedCommand(KisNodeList(), KisNodeList(),
                                                                 layer, KisNodeSP(),
                                                                 image, false));

            applicator.applyCommand(
                new SimpleRemoveLayers(KisNodeList() << layer,
                                       image),
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE);

            applicator.applyCommand(new KeepNodesSelectedCommand(KisNodeList(), KisNodeList(),
                                                                 KisNodeSP(), prevLayer,
                                                                 image, true));
        }

        applicator.end();
    }

    bool checkIsChildOf(KisNodeSP node, const KisNodeList &parents)
    {
        KisNodeList nodeParents;

        KisNodeSP parent = node->parent();
        while (parent) {
            nodeParents << parent;
            parent = parent->parent();
        }

        foreach(KisNodeSP perspectiveParent, parents) {
            if (nodeParents.contains(perspectiveParent)) {
                return true;
            }
        }

        return false;
    }

    bool checkIsCloneOf(KisNodeSP node, const KisNodeList &nodes)
    {
        bool result = false;

        KisCloneLayer *clone = dynamic_cast<KisCloneLayer*>(node.data());
        if (clone) {
            KisNodeSP cloneSource = KisNodeSP(clone->copyFrom());

            Q_FOREACH(KisNodeSP subtree, nodes) {
                result =
                    recursiveFindNode(subtree,
                                      [cloneSource](KisNodeSP node) -> bool
                                      {
                                          return node == cloneSource;
                                      });

                if (!result) {
                    result = checkIsCloneOf(cloneSource, nodes);
                }

                if (result) {
                    break;
                }
            }
        }

        return result;
    }

    void filterMergableNodes(KisNodeList &nodes, bool allowMasks)
    {
        KisNodeList::iterator it = nodes.begin();

        while (it != nodes.end()) {
            if ((!allowMasks && !qobject_cast<KisLayer*>(it->data())) ||
                checkIsChildOf(*it, nodes)) {
                //qDebug() << "Skipping node" << ppVar((*it)->name());
                it = nodes.erase(it);
            } else {
                ++it;
            }
        }
    }

    void sortMergableNodes(KisNodeSP root, KisNodeList &inputNodes, KisNodeList &outputNodes)
    {
        KisNodeList::iterator it = std::find(inputNodes.begin(), inputNodes.end(), root);

        if (it != inputNodes.end()) {
            outputNodes << *it;
            inputNodes.erase(it);
        }

        if (inputNodes.isEmpty()) {
            return;
        }

        KisNodeSP child = root->firstChild();
        while (child) {
            sortMergableNodes(child, inputNodes, outputNodes);
            child = child->nextSibling();
        }

        /**
         * By the end of recursion \p inputNodes must be empty
         */
        KIS_ASSERT_RECOVER_NOOP(root->parent() || inputNodes.isEmpty());
    }

    KisNodeList sortMergableNodes(KisNodeSP root, KisNodeList nodes)
    {
        KisNodeList result;
        sortMergableNodes(root, nodes, result);
        return result;
    }

    KisNodeList sortAndFilterMergableInternalNodes(KisNodeList nodes, bool allowMasks)
    {
        KIS_ASSERT_RECOVER(!nodes.isEmpty()) { return nodes; }

        KisNodeSP root;
        Q_FOREACH(KisNodeSP node, nodes) {
            KisNodeSP localRoot = node;
            while (localRoot->parent()) {
                localRoot = localRoot->parent();
            }

            if (!root) {
                root = localRoot;
            }
            KIS_ASSERT_RECOVER(root == localRoot) { return nodes; }
        }

        KisNodeList result;
        sortMergableNodes(root, nodes, result);
        filterMergableNodes(result, allowMasks);
        return result;
    }

    KisNodeList sortAndFilterAnyMergableNodesSafe(const KisNodeList &nodes, KisImageSP image) {
        KisNodeList filteredNodes = nodes;
        KisNodeList sortedNodes;

        KisLayerUtils::filterMergableNodes(filteredNodes, true);

        bool haveExternalNodes = false;
        Q_FOREACH (KisNodeSP node, nodes) {
            if (node->graphListener() != image->root()->graphListener()) {
                haveExternalNodes = true;
                break;
            }
        }

        if (!haveExternalNodes) {
            KisLayerUtils::sortMergableNodes(image->root(), filteredNodes, sortedNodes);
        } else {
            sortedNodes = filteredNodes;
        }

        return sortedNodes;
    }


    void addCopyOfNameTag(KisNodeSP node)
    {
        const QString prefix = i18n("Copy of");
        QString newName = node->name();
        if (!newName.startsWith(prefix)) {
            newName = QString("%1 %2").arg(prefix).arg(newName);
            node->setName(newName);
        }
    }

    KisNodeList findNodesWithProps(KisNodeSP root, const KoProperties &props, bool excludeRoot)
    {
        KisNodeList nodes;

        if ((!excludeRoot || root->parent()) && root->check(props)) {
            nodes << root;
        }

        KisNodeSP node = root->firstChild();
        while (node) {
            nodes += findNodesWithProps(node, props, excludeRoot);
            node = node->nextSibling();
        }

        return nodes;
    }

    KisNodeList filterInvisibleNodes(const KisNodeList &nodes, KisNodeList *invisibleNodes, KisNodeSP *putAfter)
    {
        KIS_ASSERT_RECOVER(invisibleNodes) { return nodes; }
        KIS_ASSERT_RECOVER(putAfter) { return nodes; }

        KisNodeList visibleNodes;
        int putAfterIndex = -1;

        Q_FOREACH(KisNodeSP node, nodes) {
            if (node->visible() || node->userLocked()) {
                visibleNodes << node;
            } else {
                *invisibleNodes << node;

                if (node == *putAfter) {
                    putAfterIndex = visibleNodes.size() - 1;
                }
            }
        }

        if (!visibleNodes.isEmpty() && putAfterIndex >= 0) {
            putAfterIndex = qBound(0, putAfterIndex, visibleNodes.size() - 1);
            *putAfter = visibleNodes[putAfterIndex];
        }

        return visibleNodes;
    }

    void filterUnlockedNodes(KisNodeList &nodes)
    {
        KisNodeList::iterator it = nodes.begin();

        while (it != nodes.end()) {
            if ((*it)->userLocked()) {
                it = nodes.erase(it);
            } else {
                ++it;
            }
        }
    }

    void changeImageDefaultProjectionColor(KisImageSP image, const KoColor &color)
    {
        KisImageSignalVector emitSignals;
        KisProcessingApplicator applicator(image,
                                           image->root(),
                                           KisProcessingApplicator::RECURSIVE,
                                           emitSignals,
                                           kundo2_i18n("Change projection color"),
                                           0,
                                           142857 + 1);
        applicator.applyCommand(new KisChangeProjectionColorCommand(image, color), KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
        applicator.end();
    }

    /**
     * There might be two approaches for merging multiple layers:
     *
     * 1) Consider the selected nodes as a distinct "group" and merge them
     *    as if they were isolated from the rest of the image. The key point
     *    of this approach is that the look of the image will change, when
     *    merging "weird" layers, like adjustment layers or layers with
     *    non-normal blending mode.
     *
     * 2) Merge layers in a way to keep the look of the image as unchanged as
     *    possible. With this approach one uses a few heuristics:
     *
     *       * when merging multiple layers with non-normal (but equal) blending
     *         mode, first merge these layers together using Normal blending mode,
     *         then set blending mode of the result to the original blending mode
     *
     *       * when merging multiple layers with different blending modes or
     *         layer styles, they are first rasterized, and then laid over each
     *         other with their own composite op. The blending mode of the final
     *         layer is set to Normal, so the user could clearly see that he should
     *         choose the correct blending mode.
     *
     * Krita uses the second approach: after merge operation, the image should look
     * as if nothing has happened (if it is technically possible).
     */
    void mergeMultipleLayersImpl(KisImageSP image, KisNodeList mergedNodes, KisNodeSP putAfter,
                                           bool flattenSingleLayer, const KUndo2MagicString &actionName,
                                           bool cleanupNodes = true, const QString layerName = QString())
    {
        if (!putAfter) {
            putAfter = mergedNodes.first();
        }

        filterMergableNodes(mergedNodes);
        {
            KisNodeList tempNodes;
            std::swap(mergedNodes, tempNodes);
            sortMergableNodes(image->root(), tempNodes, mergedNodes);
        }

        if (mergedNodes.size() <= 1 &&
            (!flattenSingleLayer && mergedNodes.size() == 1)) return;

        KisImageSignalVector emitSignals;
        emitSignals << ComplexNodeReselectionSignal(KisNodeSP(), KisNodeList(), KisNodeSP(), mergedNodes);



        KisNodeList originalNodes = mergedNodes;
        KisNodeList invisibleNodes;
        mergedNodes = filterInvisibleNodes(originalNodes, &invisibleNodes, &putAfter);

        if (mergedNodes.isEmpty()) return;


        // make sure we don't add the new layer into a locked group
        KIS_SAFE_ASSERT_RECOVER_RETURN(putAfter->parent());
        while (putAfter->parent() && !putAfter->parent()->isEditable()) {
            putAfter = putAfter->parent();
        }

        /**
         * We have reached the root of the layer hierarchy and didn't manage
         * to find a node that was editable enough for putting our merged
         * result into it. That whouldn't happen in normal circumstances,
         * unless the user chose to make the root layer visible and lock
         * it manually.
         */
        if (!putAfter->parent()) {
            return;
        }

        KisProcessingApplicator applicator(image, 0,
                                           KisProcessingApplicator::NONE,
                                           emitSignals,
                                           actionName);


        if (!invisibleNodes.isEmpty() && cleanupNodes) {

            /* If the putAfter node is invisible,
             * we should instead pick one of the nodes
             * to be merged to avoid a null putAfter
             * after we remove all invisible layers from
             * the image.
             * (The assumption is that putAfter is among
             * the layers to merge, so if it's invisible,
             * it's going to be removed)
             */
            if (!putAfter->visible()){
                putAfter = mergedNodes.first();
            }

            applicator.applyCommand(
                new SimpleRemoveLayers(invisibleNodes,
                                       image),
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE);
        }

        if (mergedNodes.size() > 1 || invisibleNodes.isEmpty()) {
            MergeMultipleInfoSP info(new MergeMultipleInfo(image, mergedNodes));

            // disable key strokes on all colorize masks, all onion skins on
            // paint layers and wait until update is finished with a barrier
            applicator.applyCommand(new DisableColorizeKeyStrokes(info));
            applicator.applyCommand(new DisableOnionSkins(info));
            applicator.applyCommand(new DisablePassThroughForHeadsOnly(info));
            applicator.applyCommand(new KUndo2Command(), KisStrokeJobData::BARRIER);

            applicator.applyCommand(new KeepMergedNodesSelected(info, putAfter, false));
            applicator.applyCommand(new FillSelectionMasks(info));
            applicator.applyCommand(new CreateMergedLayerMultiple(info, layerName), KisStrokeJobData::BARRIER);
            applicator.applyCommand(new DisableExtraCompositing(info));
            applicator.applyCommand(new KUndo2Command(), KisStrokeJobData::BARRIER);

            if (!info->frames.isEmpty()) {
                foreach (int frame, info->frames) {
                    applicator.applyCommand(new SwitchFrameCommand(info->image, frame, false, info->storage));

                    applicator.applyCommand(new AddNewFrame(info, frame));

                    applicator.applyCommand(new RefreshHiddenAreas(info));
                    applicator.applyCommand(new RefreshDelayedUpdateLayers(info), KisStrokeJobData::BARRIER);

                    applicator.applyCommand(new MergeLayersMultiple(info), KisStrokeJobData::BARRIER);

                    applicator.applyCommand(new SwitchFrameCommand(info->image, frame, true, info->storage));
                }
            } else {
                applicator.applyCommand(new RefreshHiddenAreas(info));
                applicator.applyCommand(new RefreshDelayedUpdateLayers(info), KisStrokeJobData::BARRIER);
                applicator.applyCommand(new MergeLayersMultiple(info), KisStrokeJobData::BARRIER);
            }

            //applicator.applyCommand(new MergeMetaData(info, strategy), KisStrokeJobData::BARRIER);
            if (cleanupNodes){
                applicator.applyCommand(new CleanUpNodes(info, putAfter),
                                            KisStrokeJobData::SEQUENTIAL,
                                        KisStrokeJobData::EXCLUSIVE);
            } else {
                applicator.applyCommand(new InsertNode(info, putAfter),
                                            KisStrokeJobData::SEQUENTIAL,
                                        KisStrokeJobData::EXCLUSIVE);
            }

            applicator.applyCommand(new KeepMergedNodesSelected(info, putAfter, true));
        }

        applicator.end();

    }

    void mergeMultipleLayers(KisImageSP image, KisNodeList mergedNodes, KisNodeSP putAfter)
    {
        mergeMultipleLayersImpl(image, mergedNodes, putAfter, false, kundo2_i18n("Merge Selected Nodes"));
    }

    void newLayerFromVisible(KisImageSP image, KisNodeSP putAfter)
    {
        KisNodeList mergedNodes;
        mergedNodes << image->root();

        mergeMultipleLayersImpl(image, mergedNodes, putAfter, true, kundo2_i18n("New From Visible"), false, i18nc("New layer created from all the visible layers", "Visible"));
    }

    struct MergeSelectionMasks : public KisCommandUtils::AggregateCommand {
        MergeSelectionMasks(MergeDownInfoBaseSP info, KisNodeSP putAfter)
            : m_info(info),
              m_putAfter(putAfter){}

        void populateChildCommands() override {
            KisNodeSP parent;
            CleanUpNodes::findPerfectParent(m_info->allSrcNodes(), m_putAfter, parent);

            KisLayerSP parentLayer;
            do {
                parentLayer = qobject_cast<KisLayer*>(parent.data());

                parent = parent->parent();
            } while(!parentLayer && parent);

            KisSelectionSP selection = new KisSelection();

            foreach (KisNodeSP node, m_info->allSrcNodes()) {
                KisMaskSP mask = dynamic_cast<KisMask*>(node.data());
                if (!mask) continue;

                selection->pixelSelection()->applySelection(
                    mask->selection()->pixelSelection(), SELECTION_ADD);
            }

            KisSelectionMaskSP mergedMask = new KisSelectionMask(m_info->image, i18n("Selection Mask"));
            mergedMask->initSelection(parentLayer);
            mergedMask->setSelection(selection);

            m_info->dstNode = mergedMask;
        }

    private:
        MergeDownInfoBaseSP m_info;
        KisNodeSP m_putAfter;
    };

    struct ActivateSelectionMask : public KisCommandUtils::AggregateCommand {
        ActivateSelectionMask(MergeDownInfoBaseSP info)
            : m_info(info) {}

        void populateChildCommands() override {
            KisSelectionMaskSP mergedMask = dynamic_cast<KisSelectionMask*>(m_info->dstNode.data());
            addCommand(new KisActivateSelectionMaskCommand(mergedMask, true));
        }

    private:
        MergeDownInfoBaseSP m_info;
    };

    bool tryMergeSelectionMasks(KisImageSP image, KisNodeList mergedNodes, KisNodeSP putAfter)
    {
        QList<KisSelectionMaskSP> selectionMasks;

        for (auto it = mergedNodes.begin(); it != mergedNodes.end(); /*noop*/) {
            KisSelectionMaskSP mask = dynamic_cast<KisSelectionMask*>(it->data());
            if (!mask) {
                it = mergedNodes.erase(it);
            } else {
                selectionMasks.append(mask);
                ++it;
            }
        }

        if (mergedNodes.isEmpty()) return false;

        KisLayerSP parentLayer = qobject_cast<KisLayer*>(selectionMasks.first()->parent().data());
        KIS_ASSERT_RECOVER(parentLayer) { return 0; }

        KisImageSignalVector emitSignals;

        KisProcessingApplicator applicator(image, 0,
                                           KisProcessingApplicator::NONE,
                                           emitSignals,
                                           kundo2_i18n("Merge Selection Masks"));

        MergeMultipleInfoSP info(new MergeMultipleInfo(image, mergedNodes));


        applicator.applyCommand(new MergeSelectionMasks(info, putAfter));
        applicator.applyCommand(new CleanUpNodes(info, putAfter),
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);
        applicator.applyCommand(new ActivateSelectionMask(info));
        applicator.end();

        return true;
    }

    void flattenLayer(KisImageSP image, KisLayerSP layer)
    {
        if (!layer->childCount() && !layer->layerStyle())
            return;

        KisNodeList mergedNodes;
        mergedNodes << layer;

        mergeMultipleLayersImpl(image, mergedNodes, layer, true, kundo2_i18n("Flatten Layer"));
    }

    void flattenImage(KisImageSP image, KisNodeSP activeNode)
    {
        if (!activeNode) {
            activeNode = image->root()->lastChild();
        }


        KisNodeList mergedNodes;
        mergedNodes << image->root();

        mergeMultipleLayersImpl(image, mergedNodes, activeNode, true, kundo2_i18n("Flatten Image"));
    }

    KisSimpleUpdateCommand::KisSimpleUpdateCommand(KisNodeList nodes, bool finalize, KUndo2Command *parent)
        : FlipFlopCommand(finalize, parent),
          m_nodes(nodes)
    {
    }
    void KisSimpleUpdateCommand::partB()
    {
        updateNodes(m_nodes);
    }
    void KisSimpleUpdateCommand::updateNodes(const KisNodeList &nodes)
    {
        Q_FOREACH(KisNodeSP node, nodes) {
            node->setDirty(node->extent());
        }
    }

    KisNodeSP recursiveFindNode(KisNodeSP node, std::function<bool(KisNodeSP)> func)
    {
        if (func(node)) {
            return node;
        }

        node = node->firstChild();
        while (node) {
            KisNodeSP resultNode = recursiveFindNode(node, func);
            if (resultNode) {
                return resultNode;
            }
            node = node->nextSibling();
        }

        return 0;
    }

    KisNodeSP findNodeByUuid(KisNodeSP root, const QUuid &uuid)
    {
        return recursiveFindNode(root,
            [uuid] (KisNodeSP node) {
                return node->uuid() == uuid;
        });
    }

    void forceAllDelayedNodesUpdate(KisNodeSP root)
    {
        KisLayerUtils::recursiveApplyNodes(root,
        [] (KisNodeSP node) {
            KisDelayedUpdateNodeInterface *delayedUpdate =
                    dynamic_cast<KisDelayedUpdateNodeInterface*>(node.data());
            if (delayedUpdate) {
                delayedUpdate->forceUpdateTimedNode();
            }
        });
    }

    bool hasDelayedNodeWithUpdates(KisNodeSP root)
    {
        return recursiveFindNode(root,
            [] (KisNodeSP node) {
                KisDelayedUpdateNodeInterface *delayedUpdate =
                    dynamic_cast<KisDelayedUpdateNodeInterface*>(node.data());

                return delayedUpdate ? delayedUpdate->hasPendingTimedUpdates() : false;
            });
    }

    void forceAllHiddenOriginalsUpdate(KisNodeSP root)
    {
        KisLayerUtils::recursiveApplyNodes(root,
        [] (KisNodeSP node) {
            KisCroppedOriginalLayerInterface *croppedUpdate =
                    dynamic_cast<KisCroppedOriginalLayerInterface*>(node.data());
            if (croppedUpdate) {
                croppedUpdate->forceUpdateHiddenAreaOnOriginal();
            }
        });
    }

    KisImageSP findImageByHierarchy(KisNodeSP node)
    {
        while (node) {
            const KisLayer *layer = dynamic_cast<const KisLayer*>(node.data());
            if (layer) {
                return layer->image();
            }

            node = node->parent();
        }

        return 0;
    }

    namespace Private {
    QRect realNodeChangeRect(KisNodeSP rootNode, QRect currentRect = QRect()) {
        KisNodeSP node = rootNode->firstChild();

        while(node) {
            currentRect |= realNodeChangeRect(node, currentRect);
            node = node->nextSibling();
        }

        if (!rootNode->isFakeNode()) {
            // TODO: it would be better to count up changeRect inside
            // node's extent() method
            //
            // NOTE: when flattening a group layer, we should take the change rect of the
            // all the child layers as the source of the change. We are calculating
            // the change rect **before** the update itself, therefore rootNode->exactBounds()
            // is not yet prepared, hence its exact bounds still contail old values.
            currentRect |= rootNode->projectionPlane()->changeRect(rootNode->exactBounds() | currentRect);
        }

        return currentRect;
    }
    }

    void refreshHiddenAreaAsync(KisImageSP image, KisNodeSP rootNode, const QRect &preparedArea) {
        QRect realNodeRect = Private::realNodeChangeRect(rootNode);
        if (!preparedArea.contains(realNodeRect)) {

            QRegion dirtyRegion = realNodeRect;
            dirtyRegion -= preparedArea;

            auto rc = dirtyRegion.begin();
            while (rc != dirtyRegion.end()) {
                image->refreshGraphAsync(rootNode, *rc, realNodeRect);
                rc++;
            }
        }
    }

    QRect recursiveTightNodeVisibleBounds(KisNodeSP rootNode)
    {
        QRect exactBounds;
        recursiveApplyNodes(rootNode, [&exactBounds] (KisNodeSP node) {
            exactBounds |= node->projectionPlane()->tightUserVisibleBounds();
        });
        return exactBounds;
    }

    KisNodeSP findRoot(KisNodeSP node)
    {
        if (!node) return node;

        while (node->parent()) {
            node = node->parent();
        }
        return node;
    }

    bool canChangeImageProfileInvisibly(KisImageSP image)
    {
        int numLayers = 0;
        bool hasNonNormalLayers = false;
        bool hasTransparentLayer = false;


        recursiveApplyNodes(image->root(),
            [&numLayers, &hasNonNormalLayers, &hasTransparentLayer, image] (KisNodeSP node) {
                if (!node->inherits("KisLayer")) return;

                numLayers++;

                if (node->exactBounds().isEmpty()) return;

                // this is only an approximation! it is not exact!
                if (!hasTransparentLayer &&
                    node->exactBounds() != image->bounds()) {

                    hasTransparentLayer = true;
                }

                if (!hasNonNormalLayers &&
                    node->compositeOpId() != COMPOSITE_OVER) {

                    hasNonNormalLayers = true;
                }
            });

        return numLayers == 1 || (!hasNonNormalLayers && !hasTransparentLayer);
    }

    void splitAlphaToMask(KisImageSP image, KisNodeSP node, const QString& maskName)
    {
        SplitAlphaToMaskInfoSP info( new SplitAlphaToMaskInfo(node->image(), node, maskName) );

        KisImageSignalVector emitSignals;
        KisProcessingApplicator applicator(image, 0,
                                           KisProcessingApplicator::NONE,
                                           emitSignals,
                                           kundo2_i18n("Split Alpha into a Mask"));

        applicator.applyCommand(new SimpleAddNode(info->image, info->getMask(), info->node), KisStrokeJobData::BARRIER);
        applicator.applyCommand(new InitSplitAlphaSelectionMask(info));
        if (info->frames.count() > 0) {
            Q_FOREACH(const int& frame, info->frames) {
                applicator.applyCommand(new SwitchFrameCommand(info->image, frame, false, info->storage));
                applicator.applyCommand(new AddNewFrame(info->getMask(), frame, info->node));
                applicator.applyCommand(new SplitAlphaCommand(info), KisStrokeJobData::BARRIER);
                applicator.applyCommand(new SwitchFrameCommand(info->image, frame, true, info->storage));
            }
        } else {
            applicator.applyCommand(new SplitAlphaCommand(info), KisStrokeJobData::BARRIER);
        }
        applicator.end();
    }

    void convertToPaintLayer(KisImageSP image, KisNodeSP src)
    {
        //Initialize all operation dependencies.
        ConvertToPaintLayerInfoSP info( new ConvertToPaintLayerInfo(image, src) );

        if (!info->hasTargetNode())
            return;

        KisImageSignalVector emitSignals;
        KisProcessingApplicator applicator(image, 0, KisProcessingApplicator::NONE, emitSignals, kundo2_i18n("Convert to a Paint Layer"));

        applicator.applyCommand(new SimpleAddNode(info->image(), info->targetNode(), info->sourceNode()->parent(), info->sourceNode()), KisStrokeJobData::BARRIER);

        if (info->frames().count() > 0) {
            Q_FOREACH(const int& frame, info->frames()) {
                applicator.applyCommand(new SwitchFrameCommand(info->image(), frame, false, info->storage));
                applicator.applyCommand(new RefreshDelayedUpdateLayers(info->sourceNodes()), KisStrokeJobData::BARRIER);
                applicator.applyCommand(new RefreshHiddenAreas(info->image(), info->sourceNode()), KisStrokeJobData::BARRIER);
                applicator.applyCommand(new AddNewFrame(info->targetNode(), frame, info->sourceNode()), KisStrokeJobData::BARRIER);
                applicator.applyCommand(new UploadProjectionToFrameCommand(info->sourceNode(), info->targetNode(), frame));
                applicator.applyCommand(new SwitchFrameCommand(info->image(), frame, true, info->storage));
            }
        }

        applicator.applyCommand(new SimpleRemoveLayers(info->toRemove(), info->image()));

        applicator.end();
    }

    //===========================================================

    int fetchLayerActiveRasterFrameID(KisNodeSP node)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  -1);
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, -1);

        if (!paintDevice->keyframeChannel()) {
            return -1;
        }

        const int activeTime = paintDevice->keyframeChannel()->activeKeyframeTime();
        KisRasterKeyframeSP keyframe = paintDevice->keyframeChannel()->activeKeyframeAt<KisRasterKeyframe>(activeTime);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(keyframe, -1);

        return keyframe->frameID();
    }

    int fetchLayerActiveRasterFrameTime(KisNodeSP node)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  -1);
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, -1);

        if (!paintDevice->keyframeChannel()) {
            return -1;
        }

        return paintDevice->keyframeChannel()->activeKeyframeTime();
    }

    KisTimeSpan fetchLayerActiveRasterFrameSpan(KisNodeSP node, const int time)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  KisTimeSpan::infinite(0));
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, KisTimeSpan::infinite(0));
        if (!paintDevice->keyframeChannel()) {
            return KisTimeSpan::infinite(0);
        }

        return paintDevice->keyframeChannel()->affectedFrames(time);
    }

    QSet<int> fetchLayerIdenticalRasterFrameTimes(KisNodeSP node, const int &frameTime)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  QSet<int>());
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, QSet<int>());
        if (!paintDevice->keyframeChannel()) {
            return QSet<int>();
        }

        return paintDevice->keyframeChannel()->clonesOf(node.data(), frameTime);
    }

    /* Finds all frames matching a specific frame ID. useful to filter out duplicate frames. */
    QSet<int> fetchLayerRasterFrameTimesMatchingID(KisNodeSP node, const int frameID) {
        KIS_ASSERT(node);
        KisRasterKeyframeChannel* rasterChannel = dynamic_cast<KisRasterKeyframeChannel*>(node->getKeyframeChannel(KisKeyframeChannel::Raster.id(), false));

        if (!rasterChannel) {
            return QSet<int>();
        }

        return rasterChannel->timesForFrameID(frameID);
    }

    QSet<int> fetchLayerRasterIDsAtTimes(KisNodeSP node, const QSet<int> &times)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  QSet<int>());
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, QSet<int>());
        if (!paintDevice->keyframeChannel()) {
            return QSet<int>();
        }

        QSet<int> frameIDs;

        Q_FOREACH( const int& frame, times ) {
            KisRasterKeyframeSP raster = paintDevice->keyframeChannel()->activeKeyframeAt<KisRasterKeyframe>(frame);
            frameIDs << raster->frameID();
        }

        return frameIDs;
    }

    QSet<int> filterTimesForOnlyRasterKeyedTimes(KisNodeSP node, const QSet<int> &times)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  times);
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, times);
        if (!paintDevice->keyframeChannel()) {
            return times;
        }

        return paintDevice->keyframeChannel()->allKeyframeTimes().intersect(times);
    }

    QSet<int> fetchLayerUniqueRasterTimesMatchingIDs(KisNodeSP node, QSet<int>& frameIDs)
    {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(node,  QSet<int>());
        KisPaintDeviceSP paintDevice = node->paintDevice();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice, QSet<int>());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paintDevice->framesInterface(), QSet<int>());

        QSet<int> uniqueTimes;

        Q_FOREACH( const int& id, frameIDs) {
            QSet<int> times = fetchLayerRasterFrameTimesMatchingID(node, id);
            if (times.count() > 0) {
                uniqueTimes.insert(*times.begin());
            }
        }

        return uniqueTimes;
    }

    QSet<int> fetchUniqueFrameTimes(KisNodeSP node, QSet<int> selectedTimes)
    {
        // Convert a set of selected keyframe times into set of selected "frameIDs"...
        QSet<int> selectedFrameIDs = KisLayerUtils::fetchLayerRasterIDsAtTimes(node, selectedTimes);

        // Current frame was already filtered during filter preview in `KisFilterManager::apply`...
        // So let's remove it...
        const int currentActiveFrameID = KisLayerUtils::fetchLayerActiveRasterFrameID(node);
        selectedFrameIDs.remove(currentActiveFrameID);

        // Convert frameIDs to any arbitrary frame time associated with the frameID...
        QSet<int> uniqueFrameTimes = node->paintDevice()->framesInterface() ? KisLayerUtils::fetchLayerUniqueRasterTimesMatchingIDs(node, selectedFrameIDs) : QSet<int>();

        return uniqueFrameTimes;
    }

}
