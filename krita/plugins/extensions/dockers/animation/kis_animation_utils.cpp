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
#include "kis_image.h"
#include "kis_node.h"
#include "kis_keyframe_channel.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_global.h"



namespace KisAnimationUtils {
    const QString addFrameActionName = i18n("New Frame");
    const QString duplicateFrameActionName = i18n("Copy Frame");
    const QString removeFrameActionName = i18n("Remove Frame");
    const QString removeFramesActionName = i18n("Remove Frames");
    const QString lazyFrameCreationActionName = i18n("Auto Frame Mode");
    const QString showLayerActionName = i18n("Show in Timeline");


    bool createKeyframeLazy(KisImageSP image, KisNodeSP node, int time, bool copy) {
        KisKeyframeChannel *content =
            node->getKeyframeChannel(KisKeyframeChannel::Content.id());

        if (!content) {
            node->enableAnimation();
            content =
                node->getKeyframeChannel(KisKeyframeChannel::Content.id());
            if (!content) return false;
        }

        if (copy) {
            if (content->keyframeAt(time)) return false;

            KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Copy Keyframe"));
            KisKeyframeSP srcFrame = content->activeKeyframeAt(time);

            content->copyKeyframe(srcFrame, time, cmd);
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
        } else {
            if (content->keyframeAt(time)) return false;

            KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Add Keyframe"));
            content->addKeyframe(time, cmd);
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

        foreach (const FrameItem &item, frames) {
            const int time = item.time;
            KisNodeSP node = item.node;

            KisKeyframeChannel *content =
                node->getKeyframeChannel(KisKeyframeChannel::Content.id());

            if (!content) continue;

            KisKeyframeSP keyframe = content->keyframeAt(time);
            if (!keyframe) continue;

            content->deleteKeyframe(keyframe, cmd.data());

            result = true;
        }

        if (result) {
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd.take()));
        }

        return result;
    }

    bool removeKeyframe(KisImageSP image, KisNodeSP node, int time) {
        QVector<FrameItem> frames;
        frames << FrameItem(node, time);
        return removeKeyframes(image, frames);
    }

    bool moveKeyframes(KisImageSP image,
                       const FrameItemList &srcFrames,
                       const FrameItemList &dstFrames,
                       bool copy) {

        if (srcFrames.size() != dstFrames.size()) return false;

        bool result = false;

        QScopedPointer<KUndo2Command> cmd(
            new KUndo2Command(!copy ?
                              kundo2_i18np("Move Keyframe",
                                           "Move Keyframes",
                                           srcFrames.size()) :
                              kundo2_i18np("Copy Keyframe",
                                           "Copy Keyframes",
                                           srcFrames.size()))); // lisp-lovers present ;)

        for (int i = 0; i < srcFrames.size(); i++) {
            const int srcTime = srcFrames[i].time;
            KisNodeSP srcNode = srcFrames[i].node;

            const int dstTime = dstFrames[i].time;
            KisNodeSP dstNode = dstFrames[i].node;

            if (srcNode != dstNode) continue;

            KisKeyframeChannel *content =
                srcNode->getKeyframeChannel(KisKeyframeChannel::Content.id());

            if (!content) continue;

            KisKeyframeSP dstKeyframe = content->keyframeAt(dstTime);
            if (dstKeyframe) {
                content->deleteKeyframe(dstKeyframe, cmd.data());
            }

            KisKeyframeSP srcKeyframe = content->keyframeAt(srcTime);
            if (srcKeyframe) {
                if (copy) {
                    content->copyKeyframe(srcKeyframe, dstTime, cmd.data());
                } else {
                    content->moveKeyframe(srcKeyframe, dstTime, cmd.data());
                }
            }

            result = true;
        }

        if (result) {
            image->postExecutionUndoAdapter()->addCommand(toQShared(cmd.take()));
        }

        return result;
    }

    bool moveKeyframe(KisImageSP image, KisNodeSP node, int srcTime, int dstTime) {
        QVector<FrameItem> srcFrames;
        srcFrames << FrameItem(node, srcTime);

        QVector<FrameItem> dstFrames;
        dstFrames << FrameItem(node, dstTime);

        return moveKeyframes(image, srcFrames, dstFrames);
    }
}

