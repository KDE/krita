/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_layer_utils.h"

#include "kis_image.h"
#include "kis_node.h"
#include "kis_layer.h"
#include "kis_clone_layer.h"
#include "kis_selection_mask.h"
#include "kis_meta_data_merge_strategy.h"
#include <kundo2command.h>
#include "commands/kis_image_layer_add_command.h"
#include "commands/kis_image_layer_remove_command.h"
#include "kis_abstract_projection_plane.h"
#include "kis_processing_applicator.h"
#include "kis_image_animation_interface.h"
#include "kis_keyframe_channel.h"
#include "kis_command_utils.h"

namespace KisLayerUtils {

    void fetchSelectionMasks(QList<KisNodeSP> mergedNodes, QVector<KisSelectionMaskSP> &selectionMasks)
    {
        foreach (KisNodeSP node, mergedNodes) {
            KisLayerSP layer = dynamic_cast<KisLayer*>(node.data());

            KisSelectionMaskSP mask;

            if (layer && (mask = layer->selectionMask())) {
                selectionMasks.append(mask);
            }
        }
    }

    struct MergeDownInfo {
        MergeDownInfo(KisImageSP _image,
                      KisLayerSP _prevLayer,
                      KisLayerSP _currLayer)
            : image(_image),
              prevLayer(_prevLayer),
              currLayer(_currLayer),
              savedSwitchedTime(0)
        {
            frames =
                fetchLayerFramesRecursive(prevLayer) |
                fetchLayerFramesRecursive(currLayer);
        }

        KisImageWSP image;

        KisLayerSP prevLayer;
        KisLayerSP currLayer;

        QVector<KisSelectionMaskSP> selectionMasks;

        KisLayerSP dstLayer;

        int savedSwitchedTime;
        QSet<int> frames;
    };

    typedef QSharedPointer<MergeDownInfo> MergeDownInfoSP;

    struct FillSelectionMasks : public KUndo2Command {
        FillSelectionMasks(MergeDownInfoSP info) : m_info(info) {}

        void redo() {
            QList<KisNodeSP> mergedNodes;
            mergedNodes << m_info->currLayer;
            mergedNodes << m_info->prevLayer;
            fetchSelectionMasks(mergedNodes, m_info->selectionMasks);
        }

    private:
        MergeDownInfoSP m_info;
    };

    struct RefreshHiddenAreas : public KUndo2Command {
        RefreshHiddenAreas(MergeDownInfoSP info) : m_info(info) {}

        void redo() {
            KisImageAnimationInterface *interface = m_info->image->animationInterface();
            const QRect preparedRect = !interface->externalFrameActive() ?
                m_info->image->bounds() : QRect();

            refreshHiddenAreaAsync(m_info->currLayer, preparedRect);
            refreshHiddenAreaAsync(m_info->prevLayer, preparedRect);
        }

    private:
        QRect realNodeExactBounds(KisNodeSP rootNode, QRect currentRect = QRect()) {
            KisNodeSP node = rootNode->firstChild();

            while(node) {
                currentRect |= realNodeExactBounds(node, currentRect);
                node = node->nextSibling();
            }

            // TODO: it would be better to count up changeRect inside
            // node's extent() method
            currentRect |= rootNode->projectionPlane()->changeRect(rootNode->exactBounds());

            return currentRect;
        }

        void refreshHiddenAreaAsync(KisNodeSP rootNode, const QRect &preparedArea) {
            QRect realNodeRect = realNodeExactBounds(rootNode);
            if (!preparedArea.contains(realNodeRect)) {

                QRegion dirtyRegion = realNodeRect;
                dirtyRegion -= preparedArea;

                foreach(const QRect &rc, dirtyRegion.rects()) {
                    m_info->image->refreshGraphAsync(rootNode, rc, realNodeRect);
                }
            }
        }
    private:
        MergeDownInfoSP m_info;
    };

    struct CreateMergedLayer : public KUndo2Command {
        CreateMergedLayer(MergeDownInfoSP info) : m_info(info) {}

        void redo() {
            // actual merging done by KisLayer::createMergedLayer (or specialized decendant)
            m_info->dstLayer = m_info->currLayer->createMergedLayerTemplate(m_info->prevLayer);

            if (m_info->frames.size() > 0) {
                m_info->dstLayer->enableAnimation();
            }
        }

    private:
        MergeDownInfoSP m_info;
    };

    struct MergeLayers : public KUndo2Command {
        MergeLayers(MergeDownInfoSP info) : m_info(info) {}

        void redo() {
            // actual merging done by KisLayer::createMergedLayer (or specialized decendant)
            m_info->currLayer->fillMergedLayerTemplate(m_info->dstLayer, m_info->prevLayer);
        }

    private:
        MergeDownInfoSP m_info;
    };

    struct MergeMetaData : public KUndo2Command {
        MergeMetaData(MergeDownInfoSP info, const KisMetaData::MergeStrategy* strategy)
            : m_info(info),
              m_strategy(strategy) {}

        void redo() {
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
            m_strategy->merge(m_info->dstLayer->metaData(), srcs, scores);
        }

    private:
        MergeDownInfoSP m_info;
        const KisMetaData::MergeStrategy *m_strategy;
    };

    struct CleanUpNodes : public KisCommandUtils::AggregateCommand {
        CleanUpNodes(MergeDownInfoSP info) : m_info(info) {}

        void populateChildCommands() {
            addCommand(new KisImageLayerAddCommand(m_info->image,
                                                   m_info->dstLayer,
                                                   m_info->currLayer->parent(),
                                                   m_info->currLayer,
                                                   true, false));

            safeRemoveTwoNodes(m_info->currLayer, m_info->prevLayer);
        }

    private:
        /**
         * The removal of two nodes in one go may be a bit tricky, because one
         * of them may be the clone of another. If we remove the source of a
         * clone layer, it will reincarnate into a paint layer. In this case
         * the pointer to the second layer will be lost.
         *
         * That's why we need to care about the order of the nodes removal:
         * the clone --- first, the source --- last.
         */
        void safeRemoveTwoNodes(KisNodeSP node1, KisNodeSP node2) {
            KisCloneLayer *clone1 = dynamic_cast<KisCloneLayer*>(node1.data());

            if (clone1 && KisNodeSP(clone1->copyFrom()) == node2) {
                addCommand(new KisImageLayerRemoveCommand(m_info->image, node1, false, true));
                addCommand(new KisImageLayerRemoveCommand(m_info->image, node2, false, true));
            } else {
                addCommand(new KisImageLayerRemoveCommand(m_info->image, node2, false, true));
                addCommand(new KisImageLayerRemoveCommand(m_info->image, node1, false, true));
            }
        }

    private:
        MergeDownInfoSP m_info;
    };

    struct SwitchFrameCommand : public KUndo2Command {
        SwitchFrameCommand(MergeDownInfoSP info, int time, bool finalize)
            : m_info(info),
              m_newTime(time),
              m_finalize(finalize) {}

        void redo() {
            if (!m_finalize) {
                init();
            } else {
                end();
            }
        }

        void undo() {
            if (m_finalize) {
                init();
            } else {
                end();
            }
        }

    private:
        void init() {
            KisImageAnimationInterface *interface = m_info->image->animationInterface();
            interface->image()->disableUIUpdates();
            interface->saveAndResetCurrentTime(m_newTime, &m_info->savedSwitchedTime);
        }

        void end() {
            KisImageAnimationInterface *interface = m_info->image->animationInterface();
            interface->restoreCurrentTime(&m_info->savedSwitchedTime);
            interface->image()->enableUIUpdates();
        }
    private:
        MergeDownInfoSP m_info;
        int m_newTime;
        bool m_finalize;
    };

    struct AddNewFrame : public KisCommandUtils::AggregateCommand {
        AddNewFrame(MergeDownInfoSP info, int frame) : m_info(info), m_frame(frame) {}

        void populateChildCommands() {
            KUndo2Command *cmd = new KisCommandUtils::SkipFirstRedoWrapper();
            KisKeyframeChannel *channel = m_info->dstLayer->getKeyframeChannel(KisKeyframeChannel::Content.id());
            channel->addKeyframe(m_frame, cmd);

            addCommand(cmd);
        }

    private:
        MergeDownInfoSP m_info;
        int m_frame;
    };

    QSet<int> fetchLayerFrames(KisNodeSP node) {
        KisKeyframeChannel *channel = node->getKeyframeChannel(KisKeyframeChannel::Content.id());
        if (!channel) return QSet<int>();

        return channel->allKeyframeIds();
    }

    QSet<int> fetchLayerFramesRecursive(KisNodeSP rootNode) {
        QSet<int> frames = fetchLayerFrames(rootNode);

        KisNodeSP node = rootNode->firstChild();
        while(node) {
            frames |= fetchLayerFramesRecursive(node);
            node = node->nextSibling();
        }

        return frames;
    }

    void mergeDown(KisImageSP image, KisLayerSP layer, const KisMetaData::MergeStrategy* strategy)
    {
        if (!layer->prevSibling()) return;

        // XXX: this breaks if we allow free mixing of masks and layers
        KisLayerSP prevLayer = dynamic_cast<KisLayer*>(layer->prevSibling().data());
        if (!prevLayer) return;

        KisImageSignalVector emitSignals;
        emitSignals << ModifiedSignal;

        KisProcessingApplicator applicator(image, 0,
                                           KisProcessingApplicator::NONE,
                                           emitSignals,
                                           kundo2_i18n("Merge Down"));

        MergeDownInfoSP info(new MergeDownInfo(image, prevLayer, layer));

        applicator.applyCommand(new FillSelectionMasks(info));
        applicator.applyCommand(new CreateMergedLayer(info), KisStrokeJobData::BARRIER);

        if (info->frames.size() > 0) {
            KisImageAnimationInterface *interface = info->image->animationInterface();
            const int currentTime = interface->currentTime();

            foreach (int frame, info->frames) {
                if (frame != currentTime) {
                    applicator.applyCommand(new SwitchFrameCommand(info, frame, false));
                }

                applicator.applyCommand(new AddNewFrame(info, frame));
                applicator.applyCommand(new RefreshHiddenAreas(info));
                applicator.applyCommand(new MergeLayers(info), KisStrokeJobData::BARRIER);

                if (frame != currentTime) {
                    applicator.applyCommand(new SwitchFrameCommand(info, frame, true));
                }
            }
        } else {
            applicator.applyCommand(new RefreshHiddenAreas(info));
            applicator.applyCommand(new MergeLayers(info), KisStrokeJobData::BARRIER);
        }

        applicator.applyCommand(new MergeMetaData(info, strategy), KisStrokeJobData::BARRIER);
        applicator.applyCommand(new CleanUpNodes(info),
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);

        applicator.end();
    }

}
