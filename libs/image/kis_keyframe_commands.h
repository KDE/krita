/*
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

#ifndef KIS_KEYFRAME_COMMANDS_H
#define KIS_KEYFRAME_COMMANDS_H

#include "kis_keyframe_channel.h"
#include "kundo2command.h"
#include "kritaimage_export.h"

/**
 * Places the keyframe on its channel at the specified time.
 * If the time is negative, the keyframe is removed from the channel.
 * Otherwise, the keyframe is moved within or inserted onto the channel.
 * Any overwritten keyframe will be restored on undo().
 */
class KRITAIMAGE_EXPORT KisReplaceKeyframeCommand : public KUndo2Command
{
public:
    KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel *m_channel;
    KisKeyframeBaseSP m_keyframe;
    KisKeyframeBaseSP m_overwrittenKeyframe;
    int m_oldTime{-1};
    int m_newTime;

    void moveKeyframeTo(int dstTime);
};

class KRITAIMAGE_EXPORT KisSwapFramesCommand : public KUndo2Command
{
public:
    KisSwapFramesCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP lhsFrame, KisKeyframeBaseSP rhsFrame, KUndo2Command *parentCommand);

    void redo() override;
    void undo() override;

private:
    KisKeyframeChannel *m_channel;
    KisKeyframeBaseSP m_lhsFrame;
    KisKeyframeBaseSP m_rhsFrame;
};

#endif
