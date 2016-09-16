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

#include "kis_animation_utils.h"

#include "kundo2command.h"
#include "kis_algebra_2d.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_keyframe_channel.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_global.h"
#include "kis_tool_utils.h"
#include "kis_image_animation_interface.h"



namespace KisAnimationUtils {
    const QString addFrameActionName = i18n("New Frame");
    const QString duplicateFrameActionName = i18n("Copy Frame");
    const QString removeFrameActionName = i18n("Remove Frame");
    const QString removeFramesActionName = i18n("Remove Frames");
    const QString lazyFrameCreationActionName = i18n("Auto Frame Mode");
    const QString dropFramesActionName = i18n("Drop Frames");
    const QString showLayerActionName = i18n("Show in Timeline");

    const QString newLayerActionName = i18n("New Layer");
    const QString addExistingLayerActionName = i18n("Add Existing Layer");
    const QString removeLayerActionName = i18n("Remove Layer");

    const QString addOpacityKeyframeActionName = i18n("Add opacity keyframe");
    const QString addTransformKeyframeActionName = i18n("Add transform keyframe");
    const QString removeOpacityKeyframeActionName = i18n("Remove opacity keyframe");
    const QString removeTransformKeyframeActionName = i18n("Remove transform keyframe");

    bool createKeyframeLazy(KisImageSP image, KisNodeSP node, const QString &channelId, int time, bool copy) {
        KisKeyframeChannel *channel = node->getKeyframeChannel(channelId);
        bool createdChannel = false;

        if (!channel) {
            node->enableAnimation();
            channel = node->getKeyframeChannel(channelId, true);
            if (!channel) return false;

            createdChannel = true;
        }

        if (copy) {
            if (channel->keyframeAt(time)) return false;

            KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Copy Keyframe"));
            KisKeyframeSP srcFrame = channel->activeKeyframeAt(time);

            channel->copyKeyframe(srcFrame, time, cmd);
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
        } else {
            if (channel->keyframeAt(time)) {

                if (createdChannel) return false;

                if (image->animationInterface()->currentTime() == time && channelId == KisKeyframeChannel::Content.id()) {
                    //shortcut: clearing the image instead

                    if (KisToolUtils::clearImage(image, node, 0)) {
                        return true;
                    }
                }
                //fallback: erasing the keyframe and creating it again
            }
            KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Add Keyframe"));
            channel->addKeyframe(time, cmd);
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
        }

        return true;
    }

    bool removeKeyframes(KisImageSP image, const FrameItemList &frames) {
        bool result = false;

        QScopedPointer<KUndo2Command> cmd(
            new KUndo2Command(kundo2_i18np("Remove Keyframe",
                                           "Remove Keyframes",
                                           frames.size()))); // lisp-lovers present ;)

        Q_FOREACH (const FrameItem &item, frames) {
            const int time = item.time;
            KisNodeSP node = item.node;

            KisKeyframeChannel *channel = node->getKeyframeChannel(item.channel);

            if (!channel) continue;

            KisKeyframeSP keyframe = channel->keyframeAt(time);
            if (!keyframe) continue;

            channel->deleteKeyframe(keyframe, cmd.data());

            result = true;
        }

        if (result) {
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd.take()));
        }

        return result;
    }

    bool removeKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int time) {
        QVector<FrameItem> frames;
        frames << FrameItem(node, channel, time);
        return removeKeyframes(image, frames);
    }


    struct LessOperator {
        LessOperator(const QPoint &offset)
            : m_columnCoeff(-KisAlgebra2D::signPZ(offset.x())),
              m_rowCoeff(-1000000 * KisAlgebra2D::signZZ(offset.y()))
        {
        }

        bool operator()(const QModelIndex &lhs, const QModelIndex &rhs) {
            return
                m_columnCoeff * lhs.column() + m_rowCoeff * lhs.row() <
                m_columnCoeff * rhs.column() + m_rowCoeff * rhs.row();
        }

    private:
        int m_columnCoeff;
        int m_rowCoeff;
    };

    void sortPointsForSafeMove(QModelIndexList *points, const QPoint &offset)
    {
        qSort(points->begin(), points->end(), LessOperator(offset));
    }

    bool moveKeyframes(KisImageSP image,
                       const FrameItemList &srcFrames,
                       const FrameItemList &dstFrames,
                       bool copy,
                       KUndo2Command *parentCommand) {

        if (srcFrames.size() != dstFrames.size()) return false;

        bool result = false;

        QScopedPointer<KUndo2Command> cmd(
            new KUndo2Command(!copy ?
                              kundo2_i18np("Move Keyframe",
                                           "Move %1 Keyframes",
                                           srcFrames.size()) :
                              kundo2_i18np("Copy Keyframe",
                                           "Copy %1 Keyframes",
                                           srcFrames.size()),
                              parentCommand));

        for (int i = 0; i < srcFrames.size(); i++) {
            const int srcTime = srcFrames[i].time;
            KisNodeSP srcNode = srcFrames[i].node;
            KisKeyframeChannel *srcChannel = srcNode->getKeyframeChannel(srcFrames[i].channel);

            const int dstTime = dstFrames[i].time;
            KisNodeSP dstNode = dstFrames[i].node;
            KisKeyframeChannel *dstChannel = dstNode->getKeyframeChannel(dstFrames[i].channel, true);

            if (srcNode == dstNode) {
                if (!srcChannel) continue;

                KisKeyframeSP srcKeyframe = srcChannel->keyframeAt(srcTime);
                if (srcKeyframe) {
                    if (copy) {
                        srcChannel->copyKeyframe(srcKeyframe, dstTime, cmd.data());
                    } else {
                        srcChannel->moveKeyframe(srcKeyframe, dstTime, cmd.data());
                    }
                }
            } else {
                if (!srcChannel|| !dstChannel) continue;

                KisKeyframeSP srcKeyframe = srcChannel->keyframeAt(srcTime);
                if (!srcKeyframe) continue;

                dstChannel->copyExternalKeyframe(srcChannel, srcTime, dstTime, cmd.data());

                if (!copy) {
                    srcChannel->deleteKeyframe(srcKeyframe, cmd.data());
                }
            }

            result = true;
        }

        if (parentCommand) {
            cmd.take();
        } else if (result) {
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd.take()));
        }

        return result;
    }

    bool moveKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int srcTime, int dstTime) {
        QVector<FrameItem> srcFrames;
        srcFrames << FrameItem(node, channel, srcTime);

        QVector<FrameItem> dstFrames;
        dstFrames << FrameItem(node, channel, dstTime);

        return moveKeyframes(image, srcFrames, dstFrames);
    }

    bool supportsContentFrames(KisNodeSP node)
    {
        return node->inherits("KisPaintLayer") || node->inherits("KisFilterMask") || node->inherits("KisTransparencyMask") || node->inherits("KisSelectionBasedLayer");
    }

}

