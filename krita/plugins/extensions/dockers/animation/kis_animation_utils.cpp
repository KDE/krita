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
    const QString lazyFrameCreationActionName = i18n("Auto Frame Mode");

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

    bool removeKeyframe(KisImageSP image, KisNodeSP node, int time) {
        KisKeyframeChannel *content =
            node->getKeyframeChannel(KisKeyframeChannel::Content.id());

        if (!content) return false;

        KisKeyframeSP keyframe = content->keyframeAt(time);

        if (!keyframe) return false;

        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Remove Keyframe"));
        content->deleteKeyframe(keyframe, cmd);
        image->postExecutionUndoAdapter()->addCommand(toQShared(cmd));

        return true;
    }

    bool moveKeyframe(KisImageSP image, KisNodeSP node, int srcTime, int dstTime) {
        KisKeyframeChannel *content =
            node->getKeyframeChannel(KisKeyframeChannel::Content.id());

        if (!content) return false;

        KisKeyframeSP srcKeyframe = content->keyframeAt(srcTime);
        KisKeyframeSP dstKeyframe = content->keyframeAt(dstTime);

        if (!srcKeyframe || dstKeyframe) return false;

        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Move Keyframe"));
        content->moveKeyframe(srcKeyframe, dstTime, cmd);
        image->postExecutionUndoAdapter()->addCommand(toQShared(cmd));

        return true;
    }
}

